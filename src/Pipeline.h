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
	VkPipeline getGraphicsPipeline() const { return m_GraphicsPipeline; }
	VkPipelineLayout getPipelineLayout() const { return m_PipelineLayout; }
	VkPipeline getDepthPrepassPipeline() const { return m_DepthPrepassPipeline; }
	VkPipelineLayout getDepthPrepassLayout() const { return m_DepthPrepassPipelineLayout; }

private:
	// private member variables
	Device* m_pDevice = nullptr;
	DescriptorSetLayout* m_pGlobalDescriptorSetLayout = nullptr;
	DescriptorSetLayout* m_pUboDescriptorSetLayout = nullptr;
	SwapChain* m_pSwapChain = nullptr;

	VkPipeline m_GraphicsPipeline = VK_NULL_HANDLE;
	VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_DepthPrepassPipeline = VK_NULL_HANDLE;
	VkPipelineLayout m_DepthPrepassPipelineLayout = VK_NULL_HANDLE;

	// private member functions
	void createGraphicsPipeline();
	void createDepthPrepassPipeline();
	VkShaderModule createShaderModule(const std::vector<char>& code);
};