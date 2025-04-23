#include "pch.h"
#include "Window.h"
#include "utils.h"

Window::Window(std::string title)
    : m_pTitle(title)
    , m_FramebufferResized(false) 
{
    // Initialize GLFW library
    glfwInit();

    // Tell GLFW not to create an OpenGL context & disable window resizing
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // Create a window 
    // (width, height, name, optionally specify a monitor to open window on, only relevant to OpenGL)
    m_pWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

    glfwSetWindowUserPointer(m_pWindow, this);
    glfwSetFramebufferSizeCallback(m_pWindow, framebufferResizeCallback);
}

void Window::pollEvents() const
{
    glfwPollEvents();
}

void Window::resetWindowResizedFlag() 
{
    m_FramebufferResized = false;
}

void Window::setFramebufferResizedCallback() 
{
    glfwSetFramebufferSizeCallback(m_pWindow, framebufferResizeCallback);
}

void Window::cleanupWindow()
{
    // Destroy the window
    glfwDestroyWindow(m_pWindow);

    // Terminate GLFW library
    glfwTerminate();
}

void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) 
{
    auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->m_FramebufferResized = true;
}
