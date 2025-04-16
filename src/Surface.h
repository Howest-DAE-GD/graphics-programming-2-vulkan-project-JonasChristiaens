#pragma once

class Surface
{
public:
	// constructor & destructor
	Surface(VkInstance instance, GLFWwindow* window);
	~Surface();

	// public member functions
	VkSurfaceKHR getSurface() const;
	void createSurface(VkInstance instance, GLFWwindow* window);

private:
	// private member variables
	VkSurfaceKHR m_Surface;
	VkInstance m_Instance;
};