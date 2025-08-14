#include "pch.h"
#include "DescriptorSet.h"

#include "Device.h"
#include "Texture.h"
#include "DescriptorSetLayout.h"
#include "DescriptorPool.h"
#include "Buffer.h"

#include <array>
#include <stdexcept>

DescriptorSet::DescriptorSet(Device* device, Texture* texture, DescriptorSetLayout* descriptorSetLayout, DescriptorPool* descriptorPool)
    : m_pDevice(device)
    , m_pTexture(texture)
    , m_pDescriptorSetLayout(descriptorSetLayout)
    , m_pDescriptorPool(descriptorPool)
{
}

void DescriptorSet::createDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_pDescriptorSetLayout->getDescriptorSetLayout());

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_pDescriptorPool->getDescriptorPool();
    allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
    allocInfo.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(m_pDevice->getDevice(), &allocInfo, &m_DescriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }
}

void DescriptorSet::cleanupDescriptorSet()
{
    if (m_DescriptorSet != VK_NULL_HANDLE && m_pDescriptorPool)
    {
        VkResult result = vkFreeDescriptorSets(m_pDevice->getDevice(), m_pDescriptorPool->getDescriptorPool(), 1, &m_DescriptorSet);
        m_DescriptorSet = VK_NULL_HANDLE;
    }
}

void DescriptorSet::updateGlobalDescriptorSets(uint32_t textureCount)
{
    // Existing sampler and texture array updates
    VkDescriptorImageInfo samplerInfo{};
    samplerInfo.sampler = m_pTexture->getTextureSampler();
    samplerInfo.imageView = VK_NULL_HANDLE;
    samplerInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    std::vector<VkDescriptorImageInfo> imageInfos(textureCount);
    for (uint32_t idx = 0; idx < textureCount; ++idx)
    {
        imageInfos[idx].imageView = m_pTexture->getTextureImageViews()[idx];
        imageInfos[idx].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfos[idx].sampler = VK_NULL_HANDLE;
    }

    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

    descriptorWrites[0] = {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        nullptr,
        m_DescriptorSet,
        0, // binding
        0, // arrayElement
        1, // descriptorCount
        VK_DESCRIPTOR_TYPE_SAMPLER,
        &samplerInfo,
        nullptr,
        nullptr
    };

    descriptorWrites[1] = {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        nullptr,
        m_DescriptorSet,
        1, // binding
        0, // arrayElement
        textureCount,
        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        imageInfos.data(),
        nullptr,
        nullptr
    };

    vkUpdateDescriptorSets(m_pDevice->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void DescriptorSet::updateUboDescriptorSets(std::vector<Buffer*>& uniformBuffers)
{
    for (size_t idx{}; idx < MAX_FRAMES_IN_FLIGHT; idx++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[idx]->getBuffer(); // specify buffer to bind
        // specify region within that contains data for descriptor
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_DescriptorSet; // specify descriptor set to update
        descriptorWrites[0].dstBinding = 0; // specify binding within the set
        descriptorWrites[0].dstArrayElement = 0; // specify array element within binding
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // specify type of descriptor
        descriptorWrites[0].descriptorCount = 1; // specify number of descriptors to update
        descriptorWrites[0].pBufferInfo = &bufferInfo; // specify array of descriptors

        // update descriptor set
        vkUpdateDescriptorSets(m_pDevice->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void DescriptorSet::updateGBufferDescriptorSets(VkImageView positionView, VkImageView normalView, VkImageView albedoView, VkImageView materialView) 
{
    // Validation check
    if (positionView == VK_NULL_HANDLE || normalView == VK_NULL_HANDLE ||
        albedoView == VK_NULL_HANDLE || materialView == VK_NULL_HANDLE) {
        throw std::runtime_error("One or more G-buffer image views are null!");
    }

    // Use existing sampler for all G-buffer images
    VkSampler gbufferSampler = m_pTexture->getTextureSampler();

    std::array<VkDescriptorImageInfo, 4> gBufferInfos{};

    // Position (binding 2)
    gBufferInfos[0] = {
        gbufferSampler, // sampler
        positionView,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    // Normal (binding 3)
    gBufferInfos[1] = {
        gbufferSampler, // sampler
        normalView,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    // Albedo (binding 4)
    gBufferInfos[2] = {
        gbufferSampler, // sampler
        albedoView,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    // Material (binding 5)
    gBufferInfos[3] = {
        gbufferSampler, // sampler
        materialView,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    std::array<VkWriteDescriptorSet, 4> descriptorWrites{};

    for (uint32_t i = 0; i < 4; i++) {
        descriptorWrites[i] = {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            nullptr,
            m_DescriptorSet,
            2 + i, // binding (starts at 2)
            0, // arrayElement
            1, // descriptorCount
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            &gBufferInfos[i],
            nullptr,
            nullptr
        };
    }

    vkUpdateDescriptorSets(m_pDevice->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}
