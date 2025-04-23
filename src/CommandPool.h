#pragma once

class Device;
class CommandPool
{
public:
	// constructor & destructor
	CommandPool(Device* device);
	~CommandPool() = default;
	
	// public member functions
	VkCommandPool getCommandPool() const { return m_CommandPool; }

	void destroyCommandPool();

private:
	// private member variables
	Device* m_pDevice{};
	VkCommandPool m_CommandPool{};

	// private member functions
	void createCommandPool();
};