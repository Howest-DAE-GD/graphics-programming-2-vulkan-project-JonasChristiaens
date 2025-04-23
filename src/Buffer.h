#pragma once

class Device;
class Buffer
{
public:
	// constructor & destructor
	Buffer();
	~Buffer() = default;

	// public member functions
	static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory, Device* device);

private:
	// private member variables


	// private member functions

};