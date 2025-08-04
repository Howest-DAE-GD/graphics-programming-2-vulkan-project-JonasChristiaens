#pragma once
#include <string>

class Device;
class SwapChain;
class CommandPool;
class SceneManager;
class Texture
{
public:
	// constructor & destructor
	Texture(Device* device, SwapChain* swapchain, CommandPool* commandPool, SceneManager* sceneManager);
	~Texture() = default;

	// public member functions
	void createTextureImage();
	void createTextureImageView();
	void createTextureSampler();
	void cleanupTextures();

	std::vector<VkImageView> getTextureImageViews() const { return m_TextureImageViews; }
	VkSampler getTextureSampler() const { return m_TextureSampler; }
	std::vector<uint32_t> getMipLevels() const { return m_MipLevels; }

private:
	// private member variables
	Device* m_pDevice = nullptr;
	SwapChain* m_pSwapChain = nullptr;
	CommandPool* m_pCommandPool = nullptr;
	SceneManager* m_pSceneManager = nullptr;

	std::vector<VkImage> m_TextureImages;
	std::vector<VkDeviceMemory> m_TextureImageMemories;
	std::vector<VkImageView> m_TextureImageViews;
	std::vector<uint32_t> m_MipLevels;

	VkSampler m_TextureSampler = VK_NULL_HANDLE;

	// private member functions
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
};