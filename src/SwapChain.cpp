#include "pch.h"
#include "SwapChain.h"
#include "utils.h"

#include "Device.h"
#include "Window.h"
#include "Surface.h"
#include "Renderpass.h"
#include <stdexcept>
#include <algorithm>

SwapChain::SwapChain(Device* device, Window* window, Surface* surface)
	: m_pDevice(device)
    , m_pWindow(window)
	, m_pSurface(surface)
    , m_pImage(new Image(device))
{
	createSwapChain();
    createImageViews();
}
SwapChain::~SwapChain()
{
    delete m_pImage;
}

void SwapChain::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_pDevice->getPhysicalDevice(), m_pSurface->getSurface());

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_pSurface->getSurface();

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = m_pDevice->findQueueFamilies();
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_pDevice->getDevice(), &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(m_pDevice->getDevice(), m_SwapChain, &imageCount, nullptr);
    m_SwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_pDevice->getDevice(), m_SwapChain, &imageCount, m_SwapChainImages.data());

    m_SwapChainImageFormat = surfaceFormat.format;
    m_SwapChainExtent = extent;
}
void SwapChain::cleanupSwapChain()
{
    m_pImage->cleanup();

    for (size_t i = 0; i < m_SwapChainFramebuffers.size(); i++) {
        vkDestroyFramebuffer(m_pDevice->getDevice(), m_SwapChainFramebuffers[i], nullptr);
    }

    for (size_t i = 0; i < m_SwapChainImageViews.size(); i++) {
        vkDestroyImageView(m_pDevice->getDevice(), m_SwapChainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(m_pDevice->getDevice(), m_SwapChain, nullptr);
}
void SwapChain::recreateSwapChain(Renderpass* renderpass)
{
    int width = 0, height = 0;
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(m_pWindow->getGLFWwindow(), &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(m_pDevice->getDevice());

    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createResources(renderpass);
}

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}
VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}
VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    else {
        int width, height;
        glfwGetFramebufferSize(m_pWindow->getGLFWwindow(), &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void SwapChain::createImageViews()
{
    m_SwapChainImageViews.resize(m_SwapChainImages.size());

    for (uint32_t i = 0; i < m_SwapChainImages.size(); i++) {
        m_SwapChainImageViews[i] = Image::createImageView(m_SwapChainImages[i], m_SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, m_pDevice);
    }
}
void SwapChain::createResources(Renderpass* renderpass)
{
    m_pImage->createColorResources(m_SwapChainExtent.width, m_SwapChainExtent.height, m_pDevice->getMsaaSamples(), m_SwapChainImageFormat);
    m_pImage->createDepthResources(m_SwapChainExtent.width, m_SwapChainExtent.height, m_pDevice->getMsaaSamples());
    createFramebuffers(renderpass);
}
void SwapChain::createFramebuffers(Renderpass* renderpass)
{
    m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());

    for (size_t idx = 0; idx < m_SwapChainImageViews.size(); idx++)
    {
        std::array<VkImageView, 3> attachments = {
        m_pImage->getColorImageView(),
        m_pImage->getDepthImageView(),
        m_SwapChainImageViews[idx]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderpass->getRenderPass();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_SwapChainExtent.width;
        framebufferInfo.height = m_SwapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_pDevice->getDevice(), &framebufferInfo, nullptr, &m_SwapChainFramebuffers[idx]) != VK_SUCCESS)
            throw std::runtime_error("failed to create framebuffer!");
    }
}
