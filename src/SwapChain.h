#pragma once

#include "Image.h"

class Device;
class Window;
class Surface;
class SwapChain
{
public:
	// constructor & destructor
	SwapChain(Device* device, Window* window, Surface* surface);
	~SwapChain();

	// public member functions
	void createSwapChain();
	void cleanupSwapChain();
	void recreateSwapChain();

	void createImageViews();
	void createResources();
	
	VkSwapchainKHR getSwapChain() const { return m_SwapChain; }
	VkFormat getImageFormat() const { return m_SwapChainImageFormat; }
	VkExtent2D getExtent() const { return m_SwapChainExtent; }
	const std::vector<VkImageView>& getImageViews() const { return m_SwapChainImageViews; }
	const std::vector<VkImage>& getSwapChainImages() const { return m_SwapChainImages; }

	// public member variables
	Image* m_pImage;

private:
	// private member variables
	Device* m_pDevice;
	Window* m_pWindow;
	Surface* m_pSurface;

	VkSwapchainKHR m_SwapChain{};
	std::vector<VkImage> m_SwapChainImages{};
	VkFormat m_SwapChainImageFormat{};
	VkExtent2D m_SwapChainExtent{};
	std::vector<VkImageView> m_SwapChainImageViews{};

	// private member functions
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
};