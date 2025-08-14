#include "pch.h"
#include "DescriptorPool.h"

#include "Device.h"
#include <stdexcept>

DescriptorPool::DescriptorPool(Device* device, const std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets)
	: m_pDevice(device)
{
    // pool size structure
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = maxSets; // max nr of descriptor sets that may be allocated

    if (vkCreateDescriptorPool(m_pDevice->getDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void DescriptorPool::cleanupDescriptorPool()
{
	vkDestroyDescriptorPool(m_pDevice->getDevice(), m_DescriptorPool, nullptr);
}
