#pragma once
#include <vector>

class Device;
class DescriptorSetLayout;
class SwapChain;
class Pipeline
{
public:
	// constructor & destructor
	Pipeline(Device* device, DescriptorSetLayout* GlobaldescriptorSetLayout, DescriptorSetLayout* UbodescriptorSetLayout, SwapChain* swapChain);
	~Pipeline() = default;

	// public member functions
	void cleanupPipelines();
	VkPipelineLayout getPipelineLayout() const { return m_PipelineLayout; }
	VkPipeline getGraphicsPipeline() const { return m_GraphicsPipeline; }

private:
	// private member variables
	Device* m_pDevice = nullptr;
	DescriptorSetLayout* m_pGlobalDescriptorSetLayout = nullptr;
	DescriptorSetLayout* m_pUboDescriptorSetLayout = nullptr;
	SwapChain* m_pSwapChain = nullptr;

	VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_GraphicsPipeline = VK_NULL_HANDLE;

	// private member functions
	void createGraphicsPipeline();
	VkShaderModule createShaderModule(const std::vector<char>& code);
};