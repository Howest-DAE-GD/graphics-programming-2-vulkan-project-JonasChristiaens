#include "pch.h"
#include "Surface.h"
#include "Instance.h"
#include <stdexcept>
#include <iostream>

Surface::Surface(Instance* instance, GLFWwindow* window)
	: m_pInstance(instance)
{
    std::cout << "[SURFACE] constructor" << std::endl;
}

void Surface::createSurface(VkInstance instance, GLFWwindow* window)
{

    if (m_surface != VK_NULL_HANDLE) {
        std::cout << "[Surface] Recreating surface, destroying old one: " << m_surface << std::endl;
        vkDestroySurfaceKHR(m_pInstance->getInstance(), m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }
    if (glfwCreateWindowSurface(instance, window, nullptr, &m_surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }

    std::cout << "[Surface] Creating surface: " << m_surface << std::endl;
}

void Surface::destroySurface()
{
    std::cout << "[Surface] m_surface before operation: " << m_surface << std::endl;

    if (m_surface != VK_NULL_HANDLE && m_pInstance != nullptr) {
        std::cout << "[Surface] Destroying surface: " << m_surface << std::endl;

        VkInstance instance = m_pInstance->getInstance();
        if (instance != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(instance, m_surface, nullptr);
        }
        m_surface = VK_NULL_HANDLE;
    }
    else {
        std::cout << "[Surface] Surface already destroyed or instance missing." << std::endl;
    }
}
