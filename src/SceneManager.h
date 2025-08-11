#pragma once

#include <string>
#include <vector>

class Device;
class SwapChain;
class Window;
class CommandBuffer;
class DescriptorSet;
class SceneManager
{
public:
	// constructor & destructor
	SceneManager(Device* device, SwapChain* spawChain);
	~SceneManager() = default;

	// public member functions
	uint32_t getCurrentFrame() const { return m_CurrentFrame; }

	void loadModel();
	void drawFrame(Window* window, std::vector<void*> uniformBuffersMapped, CommandBuffer* commandBuffers, DescriptorSet* globalDescriptorSet, std::vector<DescriptorSet*> uboDescriptorSets);
	void createSyncObjects();

	void cleanupScene();
	std::vector<Mesh>& getMeshes() { return m_Meshes; }
	std::vector<std::string>& getTexturePaths() { return m_texturePaths; }

private:
	// private member variables
	Device* m_pDevice{};
	SwapChain* m_pSwapChain{};

	std::vector<VkSemaphore> m_ImageAvailableSemaphores{};
	std::vector<VkSemaphore> m_RenderFinishedSemaphores{};
	std::vector<VkFence> m_InFlightFences{};
	uint32_t m_CurrentFrame{};

	std::vector<std::string> m_texturePaths{};
	std::vector<std::string> m_normalPaths{};

	std::vector<Mesh> m_Meshes{};

	// private member functions
	void updateUniformBuffer(uint32_t currentImage, std::vector<void*> uniformBuffersMapped);
};