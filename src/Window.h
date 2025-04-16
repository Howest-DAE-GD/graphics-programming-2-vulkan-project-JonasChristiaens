#pragma once
#include <string>

class Window
{
public:
	// constructor & destructor
	Window(std::string title);
	~Window();

	// public member functions
	bool shouldClose() const;
	void pollEvents() const;
	GLFWwindow* getGLFWwindow() const;

	bool wasWindowResized() const;
	void resetWindowResizedFlag();

	void setFramebufferResizedCallback();

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