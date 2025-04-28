#pragma once

#include <vector>

class Device;
class CommandPool;
class Renderpass;
class SwapChain;
class Pipeline;
class Buffer;
class DescriptorPool;
class SceneManager;
class CommandBuffer
{
public:
	// constructor & destructor
	CommandBuffer(Device* device, CommandPool* commandPool, Renderpass* renderpass, SwapChain* swapChain, Pipeline* pipeline, Buffer* vertexBuffer,
		Buffer* indexBuffer, DescriptorPool* descriptor, SceneManager* sceneManager);
	~CommandBuffer() = default;

	// public member functions
	std::vector<VkCommandBuffer> getCommandBuffers() const { return m_CommandBuffers; }

	void createCommandBuffers();
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, std::vector<uint32_t> indices);
private:
	// private member variables
	Device* m_pDevice{};
	CommandPool* m_pCommandPool{};
	Renderpass* m_pRenderpass{};
	SwapChain* m_pSwapChain{};
	Pipeline* m_pPipeline{};
	Buffer* m_pVertexBuffer{};
	Buffer* m_pIndexBuffer{};
	DescriptorPool* m_pDescriptorPool{};
	SceneManager* m_pSceneManager{};

	std::vector<VkCommandBuffer> m_CommandBuffers{};

	// private member functions

};