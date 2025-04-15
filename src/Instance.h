#pragma once
#include "stdafx.h"
#include <vector>
#include <stdexcept>

class Instance
{
public:
	Instance(const bool validationLayersEnabled, const std::vector<const char*>& validationLayers);
	~Instance();

	// public member functions
	VkInstance getInstance() const;
	VkDebugUtilsMessengerEXT getDebugMessenger() const;

private:
	// private member variables
	const bool m_ValidationLayersEnabled{};
	const std::vector<const char*> m_ValidationLayers{};

	VkInstance m_Instance{};
	VkDebugUtilsMessengerEXT m_DebugMessenger{};

	// private member functions
	void createInstance();
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();

	void setupDebugMessenger(const std::vector<const char*>& validationLayers);
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
};