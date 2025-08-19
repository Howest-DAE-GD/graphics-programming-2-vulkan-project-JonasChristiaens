#include "pch.h"
#include "DescriptorSet.h"

#include "Device.h"
#include "Texture.h"
#include "DescriptorSetLayout.h"
#include "DescriptorPool.h"
#include "Buffer.h"

#include <stdexcept>

DescriptorSet::DescriptorSet(Device* device, Texture* texture, DescriptorSetLayout* descriptorSetLayout, DescriptorPool* descriptorPool, size_t setCount)
    : m_pDevice(device)
    , m_pTexture(texture)
    , m_pDescriptorSetLayout(descriptorSetLayout)
    , m_pDescriptorPool(descriptorPool)
    , m_SetCount(setCount)
{
}

void DescriptorSet::createDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(m_SetCount, m_pDescriptorSetLayout->getDescriptorSetLayout());
    m_DescriptorSets.resize(m_SetCount);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_pDescriptorPool->getDescriptorPool();
    allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
    allocInfo.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(m_pDevice->getDevice(), &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }
}

void DescriptorSet::cleanupDescriptorSet()
{
    if (!m_DescriptorSets.empty() && m_pDescriptorPool)
    {
        vkFreeDescriptorSets(
            m_pDevice->getDevice(),
            m_pDescriptorPool->getDescriptorPool(),
            static_cast<uint32_t>(m_DescriptorSets.size()), m_DescriptorSets.data()
        );
        m_DescriptorSets.clear();
    }
}

void DescriptorSet::updateGlobalDescriptorSets(uint32_t textureCount)
{
    // Only one global set
    VkDescriptorSet dstSet = m_DescriptorSets[0];

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
        dstSet,
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
        dstSet,
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
        dstSet,
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

void DescriptorSet::updateUboDescriptorSets(const std::vector<Buffer*>& uniformBuffers)
{
    for (size_t idx = 0; idx < m_SetCount; idx++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[idx]->getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_DescriptorSets[idx];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(m_pDevice->getDevice(), 1, &descriptorWrite, 0, nullptr);
    }
}

void DescriptorSet::updateGBufferDescriptorSets(VkImageView positionView, VkImageView normalView, VkImageView albedoView, VkImageView materialView)
{
    // Only one global set
    VkDescriptorSet dstSet = m_DescriptorSets[0];

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
            dstSet,
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

void DescriptorSet::updateLightingDescriptorSet(VkImageView positionView, VkImageView normalView, VkImageView albedoView, VkImageView materialView, VkImageView depthView, VkSampler sampler, VkBuffer cameraBuffer)
{
    VkDescriptorSet dstSet = m_DescriptorSets[0];

    std::array<VkDescriptorImageInfo, 4> gBufferInfos{};
    gBufferInfos[0] = { sampler, positionView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    gBufferInfos[1] = { sampler, normalView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    gBufferInfos[2] = { sampler, albedoView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    gBufferInfos[3] = { sampler, materialView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

    std::array<VkWriteDescriptorSet, 4> descriptorWrites{};
    for (uint32_t i = 0; i < 4; i++) {
        descriptorWrites[i] = {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            nullptr,
            dstSet,
            2 + i, // bindings 2,3,4,5
            0,
            1,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            &gBufferInfos[i],
            nullptr,
            nullptr
        };
    }

    // Depth texture binding 7
    VkDescriptorImageInfo depthInfo{ sampler, depthView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    VkWriteDescriptorSet depthWrite{};
    depthWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    depthWrite.dstSet = dstSet;
    depthWrite.dstBinding = 7;
    depthWrite.descriptorCount = 1;
    depthWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    depthWrite.pImageInfo = &depthInfo;

    // Camera UBO binding 8
    VkDescriptorBufferInfo camBufferInfo{};
    camBufferInfo.buffer = cameraBuffer;
    camBufferInfo.offset = 0;
    camBufferInfo.range = sizeof(UniformBufferObject);

    VkWriteDescriptorSet camWrite{};
    camWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    camWrite.dstSet = dstSet;
    camWrite.dstBinding = 8;
    camWrite.descriptorCount = 1;
    camWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    camWrite.pBufferInfo = &camBufferInfo;

    std::array<VkWriteDescriptorSet, 6> allWrites = {
        descriptorWrites[0], descriptorWrites[1], descriptorWrites[2], descriptorWrites[3], depthWrite, camWrite
    };

    vkUpdateDescriptorSets(m_pDevice->getDevice(), static_cast<uint32_t>(allWrites.size()), allWrites.data(), 0, nullptr);
}

void DescriptorSet::updateToneMappingDescriptorSet(VkImageView hdrImageView, VkSampler sampler, VkBuffer cameraSettingsBuffer)
{
    VkDescriptorSet dstSet = m_DescriptorSets[0];

    // Binding 0: HDR image
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageView = hdrImageView;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.sampler = sampler;

    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = dstSet;
    write.dstBinding = 0;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.descriptorCount = 1;
    write.pImageInfo = &imageInfo;

    // Binding 1: CameraSettingsUBO
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = cameraSettingsBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(CameraSettingsUBO);

    VkWriteDescriptorSet writeUBO = {};
    writeUBO.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeUBO.dstSet = dstSet;
    writeUBO.dstBinding = 1;
    writeUBO.dstArrayElement = 0;
    writeUBO.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeUBO.descriptorCount = 1;
    writeUBO.pBufferInfo = &bufferInfo;

    std::array<VkWriteDescriptorSet, 2> writes = { write, writeUBO };
    vkUpdateDescriptorSets(m_pDevice->getDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}
