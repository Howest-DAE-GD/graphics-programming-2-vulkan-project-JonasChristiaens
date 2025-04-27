#include "pch.h"
#include "Buffer.h"

#include "Device.h"
#include "Commands.h"
#include "Image.h"
#include <stdexcept>

Buffer::Buffer(Device* device, CommandPool* commandPool, VkBufferUsageFlags usageFlags)
    : m_pDevice(device)
    , m_pCommandPool(commandPool)
    , m_UsageFlags(usageFlags)
{
}

void Buffer::CreateStagingBuffer(VkDeviceSize buffersize, const void* srcData)
{
    VkDeviceSize bufferSize = buffersize;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, m_pDevice);

    // Copy vertex data to the buffer
    void* data;
    vkMapMemory(m_pDevice->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data); // This function allows us to access a region of the specified memory resource defined by an offset and size
    memcpy(data, srcData, (size_t)bufferSize); // You can now simply memcpy the vertex data to the mapped memory and unmap it again using vkUnmapMemory
    vkUnmapMemory(m_pDevice->getDevice(), stagingBufferMemory);

    createBuffer(bufferSize, m_UsageFlags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Buffer, m_BufferMemory, m_pDevice);
    copyBuffer(stagingBuffer, m_Buffer, bufferSize);

    vkDestroyBuffer(m_pDevice->getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(m_pDevice->getDevice(), stagingBufferMemory, nullptr);
}

void Buffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory, Device* device)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device->getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device->getDevice(), buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = Image::findMemoryType(memRequirements.memoryTypeBits, properties, device);

    if (vkAllocateMemory(device->getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device->getDevice(), buffer, bufferMemory, 0);
}

void Buffer::cleanupBuffer()
{
    vkDestroyBuffer(m_pDevice->getDevice(), m_Buffer, nullptr);
    vkFreeMemory(m_pDevice->getDevice(), m_BufferMemory, nullptr);
}

void Buffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = Commands::beginSingleTimeCommands(m_pCommandPool, m_pDevice);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    Commands::endSingleTimeCommands(commandBuffer, m_pCommandPool, m_pDevice);
}