#include "pch.h"
#include "Surface.h"
#include <stdexcept>

Surface::Surface(VkInstance instance, GLFWwindow* window)
	: m_Instance(instance)
{
	createSurface(m_Instance, window);
}

Surface::~Surface()
{
    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
}

VkSurfaceKHR Surface::getSurface() const
{
    return m_Surface;
}

void Surface::createSurface(VkInstance instance, GLFWwindow* window)
{
    if (glfwCreateWindowSurface(instance, window, nullptr, &m_Surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}
