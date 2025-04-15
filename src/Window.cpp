#include "Window.h"

Window::Window(std::string title)
    : m_pTitle(title), 
    m_FramebufferResized(false) 
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

Window::~Window()
{
	// Destroy the window
    glfwDestroyWindow(m_pWindow);

    // Terminate GLFW library
    glfwTerminate();
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(m_pWindow);
}

void Window::pollEvents() const
{
    glfwPollEvents();
}

GLFWwindow* Window::getGLFWwindow() const
{
    return m_pWindow;
}

bool Window::wasWindowResized() const 
{
    return m_FramebufferResized;
}

void Window::resetWindowResizedFlag() 
{
    m_FramebufferResized = false;
}

void Window::setFramebufferResizedCallback() 
{
    glfwSetFramebufferSizeCallback(m_pWindow, framebufferResizeCallback);
}

void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) 
{
    auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->m_FramebufferResized = true;
}
