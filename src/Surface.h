#pragma once

class Instance;
class Surface
{
public:
	// constructor & destructor
	Surface(Instance* instance, GLFWwindow* window);
	~Surface();

	// public member functions
	VkSurfaceKHR getSurface() const { return m_Surface; }
	void createSurface(VkInstance instance, GLFWwindow* window);

private:
	// private member variables
	VkSurfaceKHR m_Surface;
	Instance* m_Instance;
};