#pragma once

class Device;
class DescriptorSetLayout
{
public:
	// constructor & destructor
	DescriptorSetLayout(Device* device);
	~DescriptorSetLayout() = default;
	
	// public member functions
	const VkDescriptorSetLayout& getDescriptorSetLayout() const { return m_DescriptorSetLayout; }
	void cleanupDescriptorSetLayout();

private:
	// private member variables
	Device* m_pDevice;
	VkDescriptorSetLayout m_DescriptorSetLayout;

	// private member functions
	void createDescriptorSetLayout();
};