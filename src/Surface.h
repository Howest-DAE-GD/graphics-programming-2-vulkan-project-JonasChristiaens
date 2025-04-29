#pragma once

class Instance;
class Surface
{
public:
	// constructor & destructor
	Surface(Instance* instance, GLFWwindow* window);
	~Surface() = default;

	// public member functions
	VkSurfaceKHR getSurface() const { return m_surface; }
	void destroySurface();

private:
	// private member variables
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	Instance* m_pInstance = nullptr;
	
	// private member functions
	void createSurface(VkInstance instance, GLFWwindow* window);
};