#pragma once
#include "utils.h"
#include <vector>

class Surface;
class Instance;
class Device
{
public:
	// constructor & destructor
	Device(Surface* surface, Instance* instance);
	~Device() = default;

	// public member functions
	VkDevice getDevice() const { return m_device; }
	VkPhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }
	VkSampleCountFlagBits getMsaaSamples() const { return m_msaaSamples; }
	VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
	VkQueue getPresentQueue() const { return m_presentQueue; }

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies();
	void cleanupDevice();
private:
	// private member variables
	Surface* m_pSurface = nullptr;
	Instance* m_pInstance = nullptr;

	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkDevice m_device = VK_NULL_HANDLE;
	VkQueue m_graphicsQueue = VK_NULL_HANDLE;
	VkQueue m_presentQueue = VK_NULL_HANDLE;


	// private member functions
	void pickPhysicalDevice();
	void createLogicalDevice();
	VkSampleCountFlagBits getMaxUsableSampleCount();

	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	bool checkSynchronization2Support(VkPhysicalDevice device);
};