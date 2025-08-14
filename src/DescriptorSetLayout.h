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

	void createGlobalDescriptorSetLayout(uint32_t textureCount, bool includeGBuffers = true);
	void createUboDescriptorSetLayout();

private:
	// private member variables
	Device* m_pDevice = nullptr;
	VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
};