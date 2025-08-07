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

void DescriptorSet::updateGlobalDescriptorSets(uint32_t textureCount)
{
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

    // binding 0: sampler
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = m_DescriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pImageInfo = &samplerInfo;

    // binding 1: sampled images
    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = m_DescriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptorWrites[1].descriptorCount = textureCount;
    descriptorWrites[1].pImageInfo = imageInfos.data();

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