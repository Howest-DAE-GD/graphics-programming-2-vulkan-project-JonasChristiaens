#include "pch.h"
#include "DescriptorPool.h"

#include "Device.h"
#include "Texture.h"
#include "DescriptorSetLayout.h"
#include "Buffer.h"
#include <stdexcept>

DescriptorPool::DescriptorPool(Device* device, Texture* texture)
	: m_pDevice(device)
    , m_pTexture(texture)
{
	createDescriptorPool();
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

void DescriptorPool::cleanupDescriptorPool()
{
	vkDestroyDescriptorPool(m_pDevice->getDevice(), m_DescriptorPool, nullptr);
}
