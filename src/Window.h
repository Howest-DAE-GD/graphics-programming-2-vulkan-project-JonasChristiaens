#pragma once
#include <string>

class Window
{
public:
	// constructor & destructor
	explicit Window(const std::string& title);
	~Window();

	// Delete copy constructor and copy assignment operator
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	// Move constructor and move assignment operator
	Window(Window&& other) noexcept;
	Window& operator=(Window&& other) noexcept;

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
	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;

	GLFWwindow* m_pWindow = nullptr;
	std::string m_title;
	bool m_framebufferResized = false;

	// private member functions
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};