#pragma once

class Device;
class Window;
class Surface;
class Renderpass;
class SwapChain
{
public:
	// constructor & destructor
	SwapChain(Device* device, Window* window, Surface* surface);
	~SwapChain() = default;

	// public member functions
	void createSwapChain();
	void cleanupSwapChain();
	void recreateSwapChain(Renderpass* renderpass);

	void createImageViews();
	void createColorResources();
	void createDepthResources();
	void createFramebuffers(Renderpass* renderpass);

	VkSwapchainKHR getSwapChain() const { return m_SwapChain; }
	VkFormat getImageFormat() const { return m_SwapChainImageFormat; }
	VkExtent2D getExtent() const { return m_SwapChainExtent; }
	const std::vector<VkImageView>& getImageViews() const { return m_SwapChainImageViews; }
	const std::vector<VkFramebuffer>& getFramebuffers() const { return m_SwapChainFramebuffers; }

	void createImage(uint32_t texWidth, uint32_t texHeight, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkFormat findDepthFormat();

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
	std::vector<VkFramebuffer> m_SwapChainFramebuffers{};

	VkImage m_ColorImage{};
	VkDeviceMemory m_ColorImageMemory{};
	VkImageView m_ColorImageView{};

	VkImage m_DepthImage{};
	VkDeviceMemory m_DepthImageMemory{};
	VkImageView m_DepthImageView{};

	// private member functions
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
};