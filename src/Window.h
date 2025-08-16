#pragma once
#include <string>

class Window
{
public:
	// constructor & destructor
	explicit Window(const std::string& title);
	~Window() = default;

	// public member functions
	void pollEvents() const;
	bool shouldClose() const { return glfwWindowShouldClose(m_pWindow); }
	GLFWwindow* getGLFWwindow() const { return m_pWindow; }

	bool wasWindowResized() const { return m_framebufferResized; }
	void resetWindowResizedFlag();

	void setFramebufferResizedCallback();

	void cleanupWindow();

private:
	// private member variables
	GLFWwindow* m_pWindow = nullptr;
	std::string m_title;
	bool m_framebufferResized = false;

	// private member functions
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};