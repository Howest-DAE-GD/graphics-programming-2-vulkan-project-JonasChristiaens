#pragma once

class Device;
class Image
{
public:
	// constructor & destructor
	Image(Device* device);
	~Image() = default;

	// public member functions
	static VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, Device* device);
	static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, Device* device);
	
	VkImageView getColorImageView() const { return m_ColorImageView; }
	VkImageView getDepthImageView() const { return m_DepthImageView; }
	VkImage getDepthImage() const { return m_DepthImage; }

	void cleanup();
	void createColorResources(uint32_t width, uint32_t height, VkSampleCountFlagBits msaaSamples, VkFormat colorFormat);
	void createDepthResources(uint32_t width, uint32_t height);
	void createImage(uint32_t texWidth, uint32_t texHeight, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	VkFormat findDepthFormat();

private:
	// private member variables
	Device* m_pDevice;

	VkImage m_ColorImage{};
	VkDeviceMemory m_ColorImageMemory{};
	VkImageView m_ColorImageView{};

	VkImage m_DepthImage{}; // niet in image
	VkDeviceMemory m_DepthImageMemory{};
	VkImageView m_DepthImageView{};

	// private member functions
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
};