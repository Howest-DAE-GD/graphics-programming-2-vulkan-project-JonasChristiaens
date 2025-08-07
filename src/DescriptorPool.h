#pragma once

class Device;
class DescriptorPool
{
public:
	// constructor & destructor
	DescriptorPool(Device* device, const std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets);
	~DescriptorPool() = default;

	// public member functions
	VkDescriptorPool getDescriptorPool() const { return m_DescriptorPool; }

	void cleanupDescriptorPool();

private:
	// private member variables
	Device* m_pDevice;
	VkDescriptorPool m_DescriptorPool{};
};