#pragma once

#include <string>
#include <vector>

class Device;
class SwapChain;
class Window;
class CommandBuffer;
class DescriptorSet;
class Buffer;
class Camera;
class Texture;

class SceneManager
{
public:
	// constructor & destructor
	SceneManager(Device* device, SwapChain* spawChain);
	~SceneManager() = default;

	// public member functions
	uint32_t getCurrentFrame() const { return m_CurrentFrame; }

	void loadModel();
	void drawFrame(Window* window, std::vector<void*> uniformBuffersMapped, CommandBuffer* commandBuffers, 
		DescriptorSet* globalDescriptorSet, DescriptorSet* uboDescriptorSet, std::vector<Buffer*>& uniformBuffers, Camera* camera, Texture* texture);
	void recreateDependentResources(DescriptorSet* globalDescriptorSet, DescriptorSet* uboDescriptorSet, std::vector<Buffer*>& uniformBuffers, Texture* texture);
	void createSyncObjects();

	void cleanupScene();
	std::vector<Mesh>& getMeshes() { return m_Meshes; }

	// Accessors for PBR texture paths
	const std::vector<std::string>& getAlbedoPaths() const { return m_albedoPaths; }
	const std::vector<std::string>& getNormalPaths() const { return m_normalPaths; }
	const std::vector<std::string>& getMetallicRoughnessPaths() const { return m_metallicRoughnessPaths; }
private:
	// private member variables
	Device* m_pDevice{};
	SwapChain* m_pSwapChain{};

	std::vector<VkSemaphore> m_ImageAvailableSemaphores{};
	std::vector<VkSemaphore> m_RenderFinishedSemaphores{};
	std::vector<VkFence> m_InFlightFences{};
	uint32_t m_CurrentFrame{};

	std::vector<Mesh> m_Meshes{};
	std::vector<std::string> m_albedoPaths;
    std::vector<std::string> m_normalPaths;
    std::vector<std::string> m_metallicRoughnessPaths;

	// private member functions
	void updateUniformBuffer(uint32_t currentImage, std::vector<void*> uniformBuffersMapped, Camera* camera);
};