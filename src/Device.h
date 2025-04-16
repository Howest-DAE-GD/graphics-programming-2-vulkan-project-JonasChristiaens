#pragma once
#include "utils.h"
#include <vector>

class Device
{
public:
	// constructor & destructor
	Device(VkSurfaceKHR surface, VkInstance instance);
	~Device();

	// public member functions
	VkDevice getDevice() const;
	VkPhysicalDevice getPhysicalDevice() const;
	VkSampleCountFlagBits getMsaaSamples() const;
	VkQueue getGraphicsQueue() const;
	VkQueue getPresentQueue() const;

private:
	// private member variables
	VkSurfaceKHR m_Surface;
	VkInstance m_Instance;

	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	VkSampleCountFlagBits m_MsaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkDevice m_Device{};
	VkQueue m_GraphicsQueue{};
	VkQueue m_PresentQueue{};

	// private member functions
	void pickPhysicalDevice();
	void createLogicalDevice();
	VkSampleCountFlagBits getMaxUsableSampleCount();

	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
};