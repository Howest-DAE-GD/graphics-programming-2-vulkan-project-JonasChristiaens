#pragma once
#include <string>

class Device;
class SwapChain;
class CommandPool;
class Texture
{
public:
	// constructor & destructor
	Texture(Device* device, SwapChain* swapchain, CommandPool* commandPool);
	~Texture() = default;

	// public member functions
	void createTextureImage();
	void createTextureImageView();
	void createTextureSampler();
	void cleanupTextures();

	VkImageView getTextureImageView() const { return m_TextureImageView; }
	VkSampler getTextureSampler() const { return m_TextureSampler; }
	uint32_t getMipLevels() const { return m_MipLevels; }

private:
	// private member variables
	Device* m_pDevice = nullptr;
	SwapChain* m_pSwapChain = nullptr;
	CommandPool* m_pCommandPool = nullptr;

	VkImage m_TextureImage = VK_NULL_HANDLE;
	VkDeviceMemory m_TextureImageMemory = VK_NULL_HANDLE;
	VkImageView m_TextureImageView = VK_NULL_HANDLE;
	VkSampler m_TextureSampler = VK_NULL_HANDLE;
	uint32_t m_MipLevels = 0;

	// private member functions
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
};