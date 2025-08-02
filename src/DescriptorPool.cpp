#include "pch.h"
#include "DescriptorPool.h"

#include "Device.h"
#include "Texture.h"
#include "DescriptorSetLayout.h"
#include "Buffer.h"
#include <stdexcept>

DescriptorPool::DescriptorPool(Device* device, Texture* texture, DescriptorSetLayout* descriptorSetLayout, std::vector<Buffer*>& uniformBuffers)
	: m_pDevice(device)
    , m_pTexture(texture)
	, m_pDescriptorSetLayout(descriptorSetLayout)
{
	createDescriptorPool();
	createDescriptorSets(uniformBuffers);
}

// Function to create pool for descriptor sets
void DescriptorPool::createDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    // pool size structure
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT); // max nr of descriptor sets that may be allocated

    if (vkCreateDescriptorPool(m_pDevice->getDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

// Function to allocate descriptor sets
void DescriptorPool::createDescriptorSets(std::vector<Buffer*>& uniformBuffers)
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_pDescriptorSetLayout->getDescriptorSetLayout());

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_DescriptorPool; // Specify pool to allocate from
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT); // Number of sets to allocate
    allocInfo.pSetLayouts = layouts.data(); // Layout to base them on

    // vector that holds the descriptor sets
    m_DescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(m_pDevice->getDevice(), &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t idx{}; idx < MAX_FRAMES_IN_FLIGHT; idx++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[idx]->getBuffer(); // specify buffer to bind
        // specify region within that contains data for descriptor
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = m_pTexture->getTextureImageViews()[idx];
        imageInfo.sampler = m_pTexture->getTextureSampler();

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_DescriptorSets[idx]; // specify descriptor set to update
        descriptorWrites[0].dstBinding = 0; // specify binding within the set
        descriptorWrites[0].dstArrayElement = 0; // specify array element within binding
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // specify type of descriptor
        descriptorWrites[0].descriptorCount = 1; // specify number of descriptors to update
        descriptorWrites[0].pBufferInfo = &bufferInfo; // specify array of descriptors

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = m_DescriptorSets[idx];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        // update descriptor set
        vkUpdateDescriptorSets(m_pDevice->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void DescriptorPool::cleanupDescriptorPool()
{
	vkDestroyDescriptorPool(m_pDevice->getDevice(), m_DescriptorPool, nullptr);
}
