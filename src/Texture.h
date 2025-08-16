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

    // Accessors
    std::vector<VkImageView> getTextureImageViews() const { return m_AlbedoImageViews; }
    std::vector<VkImageView> getNormalImageViews() const { return m_NormalImageViews; }
    VkSampler getTextureSampler() const { return m_TextureSampler; }
    std::vector<uint32_t> getMipLevels() const { return m_MipLevels; }

private:
    // private member variables
    Device* m_pDevice = nullptr;
    SwapChain* m_pSwapChain = nullptr;
    CommandPool* m_pCommandPool = nullptr;
    SceneManager* m_pSceneManager = nullptr;

    // Albedo textures
    std::vector<VkImage> m_AlbedoImages;
    std::vector<VkDeviceMemory> m_AlbedoImageMemories;
    std::vector<VkImageView> m_AlbedoImageViews;

    // Normal map textures
    std::vector<VkImage> m_NormalImages;
    std::vector<VkDeviceMemory> m_NormalImageMemories;
    std::vector<VkImageView> m_NormalImageViews;

    // Common mip levels and formats
    std::vector<uint32_t> m_MipLevels;
    std::vector<VkFormat> m_TextureFormats;

    VkSampler m_TextureSampler = VK_NULL_HANDLE;

    // private member functions
    void create1x1Fallback(unsigned char* pixel, VkFormat format, Device* device, SwapChain* swapchain, CommandPool* commandPool,
        VkImage& image, VkDeviceMemory& memory, VkImageView& imageView, uint32_t& mipLevels);
    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
};