#include "pch.h"
#include "Surface.h"
#include "Instance.h"
#include <stdexcept>

Surface::Surface(Instance* instance, GLFWwindow* window)
	: m_pInstance(instance)
{
	createSurface(m_pInstance->getInstance(), window);
}

void Surface::createSurface(VkInstance instance, GLFWwindow* window)
{
    if (glfwCreateWindowSurface(instance, window, nullptr, &m_surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

void Surface::destroySurface()
{
    if (m_surface != VK_NULL_HANDLE && m_pInstance != nullptr) {
        vkDestroySurfaceKHR(m_pInstance->getInstance(), m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }
}
