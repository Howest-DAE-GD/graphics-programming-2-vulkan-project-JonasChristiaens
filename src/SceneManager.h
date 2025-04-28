#pragma once

#include <vector>

class Device;
class SwapChain;
class Renderpass;
class Window;
class CommandBuffer;
class SceneManager
{
public:
	// constructor & destructor
	SceneManager(Device* device, SwapChain* spawChain, Renderpass* renderpass);
	~SceneManager() = default;

	// public member functions
	uint32_t getCurrentFrame() const { return m_CurrentFrame; }

	void loadModel(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
	void drawFrame(Window* window, std::vector<void*> uniformBuffersMapped, CommandBuffer* commandBuffers, std::vector<uint32_t> indices);
	void createSyncObjects();

	void cleanupScene();

private:
	// private member variables
	Device* m_pDevice{};
	SwapChain* m_pSwapChain{};
	Renderpass* m_pRenderpass{};

	std::vector<VkSemaphore> m_ImageAvailableSemaphores{};
	std::vector<VkSemaphore> m_RenderFinishedSemaphores{};
	std::vector<VkFence> m_InFlightFences{};
	uint32_t m_CurrentFrame{};

	// private member functions
	void updateUniformBuffer(uint32_t currentImage, std::vector<void*> uniformBuffersMapped);
};