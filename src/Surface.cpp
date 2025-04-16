#include "pch.h"
#include "Surface.h"
#include "Instance.h"
#include <stdexcept>

Surface::Surface(Instance* instance, GLFWwindow* window)
	: m_Instance(instance)
{
	createSurface(m_Instance->getInstance(), window);
}

Surface::~Surface()
{
    vkDestroySurfaceKHR(m_Instance->getInstance(), m_Surface, nullptr);
}

void Surface::createSurface(VkInstance instance, GLFWwindow* window)
{
    if (glfwCreateWindowSurface(instance, window, nullptr, &m_Surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}
