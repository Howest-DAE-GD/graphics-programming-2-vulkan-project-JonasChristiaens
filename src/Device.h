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
	~Device();

	// public member functions
	VkDevice getDevice() const { return m_Device; }
	VkPhysicalDevice getPhysicalDevice() const { return m_PhysicalDevice; }
	VkSampleCountFlagBits getMsaaSamples() const { return m_MsaaSamples; }
	VkQueue getGraphicsQueue() const { return m_GraphicsQueue; }
	VkQueue getPresentQueue() const { return m_PresentQueue; }

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
private:
	// private member variables
	Surface* m_Surface;
	Instance* m_Instance;

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
};