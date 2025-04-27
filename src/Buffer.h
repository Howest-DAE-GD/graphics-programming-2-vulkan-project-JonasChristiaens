#pragma once

class Device;
class CommandPool;
class Buffer
{
public:
	// constructor & destructor
	Buffer(Device* device, CommandPool* commandPool, VkBufferUsageFlags usageFlags);
	~Buffer() = default;

	// public member functions
	const VkBuffer& getBuffer() const { return m_Buffer; }
	const VkDeviceMemory getBufferMemory() const { return m_BufferMemory; }

	void CreateStagingBuffer(VkDeviceSize buffersize, const void* srcData);
	static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory, Device* device);

	void cleanupBuffer();
private:
	// private member variables
	Device* m_pDevice{};
	CommandPool* m_pCommandPool{};

	VkBuffer m_Buffer{};
	VkDeviceMemory m_BufferMemory{};
	VkBufferUsageFlags m_UsageFlags{};

	// private member functions
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
};