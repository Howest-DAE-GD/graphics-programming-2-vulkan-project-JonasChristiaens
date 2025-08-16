#include "pch.h"
#include "Texture.h"
#include "utils.h"

#include "Device.h"
#include "SwapChain.h"
#include "Buffer.h"
#include "Commands.h"
#include "SceneManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <unordered_set>

// Static fallback resources
namespace {
    VkImage fallbackAlbedoImage = VK_NULL_HANDLE, fallbackNormalImage = VK_NULL_HANDLE;
    VkDeviceMemory fallbackAlbedoMemory = VK_NULL_HANDLE, fallbackNormalMemory = VK_NULL_HANDLE;
    VkImageView fallbackAlbedoView = VK_NULL_HANDLE, fallbackNormalView = VK_NULL_HANDLE;
    uint32_t fallbackAlbedoMip = 1, fallbackNormalMip = 1;
}

Texture::Texture(Device* device, SwapChain* swapchain, CommandPool* commandPool, SceneManager* sceneManager)
    : m_pDevice(device)
    , m_pSwapChain(swapchain)
    , m_pCommandPool(commandPool)
    , m_pSceneManager(sceneManager)
{
    createTextureImage();
    createTextureImageView();
    createTextureSampler();
}

void Texture::createTextureImage()
{
    // Fallback format for albedo and normal
    const VkFormat albedoFormat = VK_FORMAT_R8G8B8A8_SRGB, normalFormat = VK_FORMAT_R8G8B8A8_UNORM;

    // Create fallback 1x1 white/albedo
    if (fallbackAlbedoImage == VK_NULL_HANDLE) {
        unsigned char white[4] = { 255,255,255,255 }; // albedo fallback: white
        create1x1Fallback(white, albedoFormat, m_pDevice, m_pSwapChain, m_pCommandPool,
            fallbackAlbedoImage, fallbackAlbedoMemory, fallbackAlbedoView, fallbackAlbedoMip);
    }
    // Create fallback 1x1 normal
    if (fallbackNormalImage == VK_NULL_HANDLE) {
        unsigned char flatNormal[4] = { 128,128,255,255 }; // normal fallback: flat Z
        create1x1Fallback(flatNormal, normalFormat, m_pDevice, m_pSwapChain, m_pCommandPool,
            fallbackNormalImage, fallbackNormalMemory, fallbackNormalView, fallbackNormalMip);
    }

    // Load albedo textures
    const auto& albedoPaths = m_pSceneManager->getAlbedoPaths();
    for (const auto& path : albedoPaths) {
        if (path.empty()) {
            m_AlbedoImages.push_back(fallbackAlbedoImage);
            m_AlbedoImageMemories.push_back(fallbackAlbedoMemory);
            m_AlbedoImageViews.push_back(fallbackAlbedoView);
            m_MipLevels.push_back(fallbackAlbedoMip);
            m_TextureFormats.push_back(albedoFormat);
            continue;
        }
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if (!pixels) throw std::runtime_error("failed to load texture image: " + path);

        VkDeviceSize imageSize = texWidth * texHeight * 4;
        uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
        m_MipLevels.push_back(mipLevels);
        m_TextureFormats.push_back(albedoFormat);

        // Create staging buffer
        VkBuffer stagingBuffer{};
        VkDeviceMemory stagingBufferMemory{};
        Buffer::createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory, m_pDevice);

        void* data;
        vkMapMemory(m_pDevice->getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(m_pDevice->getDevice(), stagingBufferMemory);

        stbi_image_free(pixels);

        // Create image and memory
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        m_pSwapChain->m_pImage->createImage(texWidth, texHeight, mipLevels,
            VK_SAMPLE_COUNT_1_BIT, albedoFormat, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

        m_AlbedoImages.push_back(textureImage);
        m_AlbedoImageMemories.push_back(textureImageMemory);

        // Copy and transitions
        transitionImageLayout(textureImage, albedoFormat,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
        copyBufferToImage(stagingBuffer, textureImage,
            static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

        vkDestroyBuffer(m_pDevice->getDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_pDevice->getDevice(), stagingBufferMemory, nullptr);

        generateMipmaps(textureImage, albedoFormat, texWidth, texHeight, mipLevels);
    }

    // Load normal textures
    const auto& normalPaths = m_pSceneManager->getNormalPaths();
    for (const auto& path : normalPaths) {
        if (path.empty()) {
            m_NormalImages.push_back(fallbackNormalImage);
            m_NormalImageMemories.push_back(fallbackNormalMemory);
            m_NormalImageViews.push_back(fallbackNormalView);
            continue;
        }
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if (!pixels) throw std::runtime_error("failed to load normal map image: " + path);

        VkDeviceSize imageSize = texWidth * texHeight * 4;
        uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

        // Create staging buffer
        VkBuffer stagingBuffer{};
        VkDeviceMemory stagingBufferMemory{};
        Buffer::createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory, m_pDevice);

        void* data;
        vkMapMemory(m_pDevice->getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(m_pDevice->getDevice(), stagingBufferMemory);

        stbi_image_free(pixels);

        // Create image and memory
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        m_pSwapChain->m_pImage->createImage(texWidth, texHeight, mipLevels,
            VK_SAMPLE_COUNT_1_BIT, normalFormat, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

        m_NormalImages.push_back(textureImage);
        m_NormalImageMemories.push_back(textureImageMemory);

        // Copy and transitions
        transitionImageLayout(textureImage, normalFormat,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
        copyBufferToImage(stagingBuffer, textureImage,
            static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

        vkDestroyBuffer(m_pDevice->getDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_pDevice->getDevice(), stagingBufferMemory, nullptr);

        generateMipmaps(textureImage, normalFormat, texWidth, texHeight, mipLevels);

        // Create image view
        VkImageView imageView = Image::createImageView(
            textureImage,
            normalFormat,
            VK_IMAGE_ASPECT_COLOR_BIT,
            mipLevels,
            m_pDevice
        );
        m_NormalImageViews.push_back(imageView);
    }
}

void Texture::createTextureImageView()
{
    // Albedo image views
    m_AlbedoImageViews.clear();
    for (size_t i = 0; i < m_AlbedoImages.size(); i++) {
        if (m_AlbedoImages[i] == VK_NULL_HANDLE) {
            throw std::runtime_error("Albedo image is VK_NULL_HANDLE, fallback not set up!");
        }
        VkImageView imageView = Image::createImageView(
            m_AlbedoImages[i],
            m_TextureFormats[i],
            VK_IMAGE_ASPECT_COLOR_BIT,
            m_MipLevels[i],
            m_pDevice
        );
        m_AlbedoImageViews.push_back(imageView);
    }

    // Normal image views already created in createTextureImage()
}

void Texture::createTextureSampler()
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_pDevice->getPhysicalDevice(), &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = m_MipLevels.empty() ? 0.0f : static_cast<float>(*std::max_element(m_MipLevels.begin(), m_MipLevels.end()));
    samplerInfo.mipLodBias = 0.0f;

    if (vkCreateSampler(m_pDevice->getDevice(), &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS)
        throw std::runtime_error("failed to create texture sampler!");
}

void Texture::cleanupTextures()
{
    // 1. Destroy sampler
    if (m_TextureSampler != VK_NULL_HANDLE) {
        vkDestroySampler(m_pDevice->getDevice(), m_TextureSampler, nullptr);
        m_TextureSampler = VK_NULL_HANDLE;
    }

    // 2. Track destroyed handles
    std::unordered_set<VkImageView> destroyedViews;
    std::unordered_set<VkImage> destroyedImages;
    std::unordered_set<VkDeviceMemory> destroyedMemory;

    // 3. Destroy albedo image views
    for (auto imageView : m_AlbedoImageViews) {
        if (imageView != VK_NULL_HANDLE && destroyedViews.count(imageView) == 0) {
            vkDestroyImageView(m_pDevice->getDevice(), imageView, nullptr);
            destroyedViews.insert(imageView);
        }
    }
    m_AlbedoImageViews.clear();

    // 4. Destroy normal image views
    for (auto imageView : m_NormalImageViews) {
        if (imageView != VK_NULL_HANDLE && destroyedViews.count(imageView) == 0) {
            vkDestroyImageView(m_pDevice->getDevice(), imageView, nullptr);
            destroyedViews.insert(imageView);
        }
    }
    m_NormalImageViews.clear();

    // 5. Destroy all albedo images and free memory
    for (size_t i = 0; i < m_AlbedoImages.size(); i++) {
        if (m_AlbedoImages[i] != VK_NULL_HANDLE && destroyedImages.count(m_AlbedoImages[i]) == 0) {
            vkDestroyImage(m_pDevice->getDevice(), m_AlbedoImages[i], nullptr);
            destroyedImages.insert(m_AlbedoImages[i]);
        }
        if (m_AlbedoImageMemories[i] != VK_NULL_HANDLE && destroyedMemory.count(m_AlbedoImageMemories[i]) == 0) {
            vkFreeMemory(m_pDevice->getDevice(), m_AlbedoImageMemories[i], nullptr);
            destroyedMemory.insert(m_AlbedoImageMemories[i]);
        }
    }
    m_AlbedoImages.clear();
    m_AlbedoImageMemories.clear();

    // 6. Destroy all normal images and free memory
    for (size_t i = 0; i < m_NormalImages.size(); i++) {
        if (m_NormalImages[i] != VK_NULL_HANDLE && destroyedImages.count(m_NormalImages[i]) == 0) {
            vkDestroyImage(m_pDevice->getDevice(), m_NormalImages[i], nullptr);
            destroyedImages.insert(m_NormalImages[i]);
        }
        if (m_NormalImageMemories[i] != VK_NULL_HANDLE && destroyedMemory.count(m_NormalImageMemories[i]) == 0) {
            vkFreeMemory(m_pDevice->getDevice(), m_NormalImageMemories[i], nullptr);
            destroyedMemory.insert(m_NormalImageMemories[i]);
        }
    }
    m_NormalImages.clear();
    m_NormalImageMemories.clear();

    m_MipLevels.clear();
    m_TextureFormats.clear();

    // 7. Destroy static fallback resources ONLY if not already destroyed
    if (fallbackAlbedoView != VK_NULL_HANDLE && destroyedViews.count(fallbackAlbedoView) == 0) {
        vkDestroyImageView(m_pDevice->getDevice(), fallbackAlbedoView, nullptr);
        fallbackAlbedoView = VK_NULL_HANDLE;
    }
    if (fallbackAlbedoImage != VK_NULL_HANDLE && destroyedImages.count(fallbackAlbedoImage) == 0) {
        vkDestroyImage(m_pDevice->getDevice(), fallbackAlbedoImage, nullptr);
        fallbackAlbedoImage = VK_NULL_HANDLE;
    }
    if (fallbackAlbedoMemory != VK_NULL_HANDLE && destroyedMemory.count(fallbackAlbedoMemory) == 0) {
        vkFreeMemory(m_pDevice->getDevice(), fallbackAlbedoMemory, nullptr);
        fallbackAlbedoMemory = VK_NULL_HANDLE;
    }
    if (fallbackNormalView != VK_NULL_HANDLE && destroyedViews.count(fallbackNormalView) == 0) {
        vkDestroyImageView(m_pDevice->getDevice(), fallbackNormalView, nullptr);
        fallbackNormalView = VK_NULL_HANDLE;
    }
    if (fallbackNormalImage != VK_NULL_HANDLE && destroyedImages.count(fallbackNormalImage) == 0) {
        vkDestroyImage(m_pDevice->getDevice(), fallbackNormalImage, nullptr);
        fallbackNormalImage = VK_NULL_HANDLE;
    }
    if (fallbackNormalMemory != VK_NULL_HANDLE && destroyedMemory.count(fallbackNormalMemory) == 0) {
        vkFreeMemory(m_pDevice->getDevice(), fallbackNormalMemory, nullptr);
        fallbackNormalMemory = VK_NULL_HANDLE;
    }
}

void Texture::create1x1Fallback(unsigned char* pixel, VkFormat format, Device* device, SwapChain* swapchain, CommandPool* commandPool, VkImage& image, VkDeviceMemory& memory, VkImageView& imageView, uint32_t& mipLevels)
{
    int texWidth = 1, texHeight = 1;
    VkDeviceSize imageSize = 4; // RGBA8

    VkBuffer stagingBuffer{};
    VkDeviceMemory stagingBufferMemory{};
    Buffer::createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory, device);

    void* data;
    vkMapMemory(device->getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixel, imageSize);
    vkUnmapMemory(device->getDevice(), stagingBufferMemory);

    mipLevels = 1;
    swapchain->m_pImage->createImage(texWidth, texHeight, mipLevels,
        VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, memory);

    transitionImageLayout(image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
    copyBufferToImage(stagingBuffer, image, texWidth, texHeight);
    transitionImageLayout(image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);

    vkDestroyBuffer(device->getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(device->getDevice(), stagingBufferMemory, nullptr);

    imageView = Image::createImageView(image, format, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, device);
}

void Texture::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
{
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(m_pDevice->getPhysicalDevice(), imageFormat, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
        throw std::runtime_error("texture image format does not support linear blitting!");

    VkCommandBuffer commandBuffer = Commands::beginSingleTimeCommands(m_pCommandPool, m_pDevice);

    VkImageMemoryBarrier2 barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < mipLevels; i++)
    {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;

        VkDependencyInfo dependencyInfo{};
        dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependencyInfo.imageMemoryBarrierCount = 1;
        dependencyInfo.pImageMemoryBarriers = &barrier;
        vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);

        VkImageBlit blit{};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(commandBuffer,
            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit,
            VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

        vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

    VkDependencyInfo dependencyInfo{};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &barrier;

    vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);

    Commands::endSingleTimeCommands(commandBuffer, m_pCommandPool, m_pDevice);
}

void Texture::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
{
    VkCommandBuffer commandBuffer = Commands::beginSingleTimeCommands(m_pCommandPool, m_pDevice);

    VkImageMemoryBarrier2 barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkDependencyInfo dependencyInfo{};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &barrier;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    }
    else
        throw std::invalid_argument("unsupported layout transition!");

    vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);

    Commands::endSingleTimeCommands(commandBuffer, m_pCommandPool, m_pDevice);
}

void Texture::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = Commands::beginSingleTimeCommands(m_pCommandPool, m_pDevice);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    Commands::endSingleTimeCommands(commandBuffer, m_pCommandPool, m_pDevice);
}