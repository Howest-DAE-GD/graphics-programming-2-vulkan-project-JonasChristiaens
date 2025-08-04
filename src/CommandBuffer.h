#pragma once

#include <vector>

class Device;
class CommandPool;
class Renderpass;
class SwapChain;
class Pipeline;
class Buffer;
class DescriptorSet;
class SceneManager;
class CommandBuffer
{
public:
	// constructor & destructor
	CommandBuffer(Device* device, CommandPool* commandPool, Renderpass* renderpass, SwapChain* swapChain, Pipeline* pipeline, std::vector<Buffer*> vertexBuffer,
		std::vector<Buffer*> indexBuffer, SceneManager* sceneManager);
	~CommandBuffer() = default;

	// public member functions
	std::vector<VkCommandBuffer> getCommandBuffers() const { return m_CommandBuffers; }

	void createCommandBuffers();
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, std::vector<Mesh> Meshes, DescriptorSet* globalDescriptorSet, std::vector<DescriptorSet*> uboDescriptorSets);
private:
	// private member variables
	Device* m_pDevice{};
	CommandPool* m_pCommandPool{};
	Renderpass* m_pRenderpass{};
	SwapChain* m_pSwapChain{};
	Pipeline* m_pPipeline{};
	std::vector<Buffer*> m_pVertexBuffers{};
	std::vector<Buffer*> m_pIndexBuffers{};
	SceneManager* m_pSceneManager{};

	std::vector<VkCommandBuffer> m_CommandBuffers{};
};