#pragma once
#include <vector>

class Instance
{
public:
	// constructor & destructor
	Instance();
	~Instance() = default;

	// public member functions
	void createInstance();
	void setupDebugMessenger(const std::vector<const char*>& validationLayers);

	VkInstance getInstance() const { return m_instance; }
	VkDebugUtilsMessengerEXT getDebugMessenger() const { return m_debugMessenger; }

	void destroyDebugUtilsMessenger();
	void destroyInstance();

private:
	// private member variables
	VkInstance m_instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;

	// private member functions
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
};