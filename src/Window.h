#pragma once
#include <string>

class Window
{
public:
	// constructor & destructor
	Window(std::string title);
	~Window() = default;

	// public member functions
	void pollEvents() const;
	bool shouldClose() const { return glfwWindowShouldClose(m_pWindow); }
	GLFWwindow* getGLFWwindow() const { return m_pWindow; }

	bool wasWindowResized() const { return m_FramebufferResized; }
	void resetWindowResizedFlag();

	void setFramebufferResizedCallback();

	void cleanupWindow();

private:
	// private member variables
	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;

	GLFWwindow* m_pWindow;
	std::string m_pTitle;
	bool m_FramebufferResized;

	// private member functions
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};