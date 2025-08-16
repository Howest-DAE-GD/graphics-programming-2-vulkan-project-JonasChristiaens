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
    std::vector<VkDescriptorSetLayout> layouts(1, m_pDescriptorSetLayout->getDescriptorSetLayout());

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_pDescriptorPool->getDescriptorPool();
    allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
    allocInfo.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(m_pDevice->getDevice(), &allocInfo, &m_DescriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }
}

void DescriptorSet::cleanupDescriptorSet()
{
    if (m_DescriptorSet != VK_NULL_HANDLE && m_pDescriptorPool)
    {
        VkResult result = vkFreeDescriptorSets(
            m_pDevice->getDevice(),
            m_pDescriptorPool->getDescriptorPool(),
            1, &m_DescriptorSet
        );
        
        m_DescriptorSet = VK_NULL_HANDLE;
    }
}

void DescriptorSet::updateGlobalDescriptorSets(uint32_t textureCount)
{
    // Binding 0: Sampler
    VkDescriptorImageInfo samplerInfo{};
    samplerInfo.sampler = m_pTexture->getTextureSampler();
    samplerInfo.imageView = VK_NULL_HANDLE;
    samplerInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // Binding 1: Albedo texture array
    std::vector<VkDescriptorImageInfo> albedoImageInfos(textureCount);
    for (uint32_t idx = 0; idx < textureCount; ++idx)
    {
        albedoImageInfos[idx].imageView = m_pTexture->getTextureImageViews()[idx];
        albedoImageInfos[idx].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        albedoImageInfos[idx].sampler = VK_NULL_HANDLE;
    }

    // Binding 6: Normal map texture array
    std::vector<VkDescriptorImageInfo> normalImageInfos(textureCount);
    for (uint32_t idx = 0; idx < textureCount; ++idx)
    {
        normalImageInfos[idx].imageView = m_pTexture->getNormalImageViews()[idx];
        normalImageInfos[idx].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        normalImageInfos[idx].sampler = VK_NULL_HANDLE;
    }

    // Descriptor writes for sampler, albedo textures, and normal maps
    std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

    // Sampler binding 0
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

    // Albedo texture array binding 1
    descriptorWrites[1] = {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        nullptr,
        m_DescriptorSet,
        1, // binding
        0, // arrayElement
        textureCount,
        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        albedoImageInfos.data(),
        nullptr,
        nullptr
    };

    // Normal map texture array binding 6
    descriptorWrites[2] = {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        nullptr,
        m_DescriptorSet,
        6, // binding
        0, // arrayElement
        textureCount,
        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        normalImageInfos.data(),
        nullptr,
        nullptr
    };

    vkUpdateDescriptorSets(m_pDevice->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void DescriptorSet::updateUboDescriptorSets(std::vector<Buffer*>& uniformBuffers)
{
    for (size_t idx{}; idx < uniformBuffers.size(); idx++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[idx]->getBuffer(); // specify buffer to bind
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_DescriptorSet;
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

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

    VkSampler gbufferSampler = m_pTexture->getTextureSampler();

    std::array<VkDescriptorImageInfo, 4> gBufferInfos{};
    gBufferInfos[0] = { gbufferSampler, positionView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    gBufferInfos[1] = { gbufferSampler, normalView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    gBufferInfos[2] = { gbufferSampler, albedoView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    gBufferInfos[3] = { gbufferSampler, materialView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

    std::array<VkWriteDescriptorSet, 4> descriptorWrites{};

    for (uint32_t i = 0; i < 4; i++) {
        descriptorWrites[i] = {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            nullptr,
            m_DescriptorSet,
            2 + i, // bindings 2,3,4,5
            0,
            1,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            &gBufferInfos[i],
            nullptr,
            nullptr
        };
    }

    vkUpdateDescriptorSets(m_pDevice->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}
