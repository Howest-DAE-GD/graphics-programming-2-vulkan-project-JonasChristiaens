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

	// Create layout for global descriptor set
	void createGlobalDescriptorSetLayout(uint32_t textureCount, bool includeGBuffers = true, bool includeCameraUBO = false);

	// Create layout for UBO descriptor set
	void createUboDescriptorSetLayout();

	// Create layout for tone mapping descriptor set
	void createToneMappingDescriptorSetLayout();

private:
	// private member variables
	Device* m_pDevice = nullptr;
	VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
};