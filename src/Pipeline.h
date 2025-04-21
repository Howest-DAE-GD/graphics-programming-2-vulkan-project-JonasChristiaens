#pragma once
#include <vector>
#include <string>

class Device;
class DescriptorSetLayout;
class Renderpass;
class Pipeline
{
public:
	// constructor & destructor
	Pipeline(Device* device, DescriptorSetLayout* descriptorSetLayout, Renderpass* renderpass);
	~Pipeline() = default;

	// public member functions
	void cleanupPipelines();
	VkPipelineLayout getPipelineLayout() const { return m_PipelineLayout; }
	VkPipeline getGraphicsPipeline() const { return m_GraphicsPipeline; }

private:
	// private member variables
	Device* m_pDevice{};
	DescriptorSetLayout* m_pDescriptorSetLayout{};
	Renderpass* m_pRenderpass{};

	VkPipelineLayout m_PipelineLayout{};
	VkPipeline m_GraphicsPipeline{};

	// private member functions
	void createGraphicsPipeline();
	static std::vector<char> readFile(const std::string& filename);
	VkShaderModule createShaderModule(const std::vector<char>& code);
};