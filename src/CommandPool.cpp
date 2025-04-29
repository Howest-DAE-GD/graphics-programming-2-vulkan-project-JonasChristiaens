#include "pch.h"
#include "CommandPool.h"

#include "Device.h"
#include <stdexcept>

CommandPool::CommandPool(Device* device)
	: m_pDevice(device)
{
	createCommandPool();
}

void CommandPool::destroyCommandPool()
{
    if (m_CommandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(m_pDevice->getDevice(), m_CommandPool, nullptr);
        m_CommandPool = VK_NULL_HANDLE;
    }
}

void CommandPool::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = m_pDevice->findQueueFamilies();

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(m_pDevice->getDevice(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}