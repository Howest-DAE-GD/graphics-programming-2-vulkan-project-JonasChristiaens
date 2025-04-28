#pragma once
#include <vector>

class Instance
{
public:
	// constructor & destructor
	Instance();
	~Instance();

	// Delete copy constructor and copy assignment operator
	Instance(const Instance&) = delete;
	Instance& operator=(const Instance&) = delete;

	// Move constructor and move assignment operator
	Instance(Instance&& other) noexcept;
	Instance& operator=(Instance&& other) noexcept;

	// public member functions
	VkInstance getInstance() const { return m_instance; }
	VkDebugUtilsMessengerEXT getDebugMessenger() const { return m_debugMessenger; }

	void destroyDebugUtilsMessenger();
	void destroyInstance();

private:
	// private member variables
	VkInstance m_instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;

	// private member functions
	void createInstance();
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();

	void setupDebugMessenger(const std::vector<const char*>& validationLayers);
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
};