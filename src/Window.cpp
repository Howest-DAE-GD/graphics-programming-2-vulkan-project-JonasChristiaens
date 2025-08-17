#include "pch.h"
#include "Window.h"
#include "utils.h"

#include <stdexcept>
#include <iostream>

Window::Window(const std::string& title)
    : m_title(title)
{
    // Initialize GLFW library
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    // Tell GLFW not to create an OpenGL context & disable window resizing
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // Create a window 
    m_pWindow = glfwCreateWindow(WIDTH, HEIGHT, m_title.c_str(), nullptr, nullptr);
    if (!m_pWindow) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    std::cout << "[Window] Creating window: " << m_pWindow << std::endl;

    glfwSetWindowUserPointer(m_pWindow, this);
    glfwSetFramebufferSizeCallback(m_pWindow, framebufferResizeCallback);
}

void Window::pollEvents() const
{
    glfwPollEvents();
}

void Window::resetWindowResizedFlag()
{
    m_framebufferResized = false;
}

void Window::setFramebufferResizedCallback()
{
    glfwSetFramebufferSizeCallback(m_pWindow, framebufferResizeCallback);
}

void Window::cleanupWindow()
{
    if (m_pWindow)
    {
        std::cout << "[Window] destroying window: " << m_pWindow << std::endl;

        // Destroy the window
        glfwDestroyWindow(m_pWindow);
        m_pWindow = nullptr;
    }

    // Terminate GLFW library
    glfwTerminate();
}

void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->m_framebufferResized = true;
}
