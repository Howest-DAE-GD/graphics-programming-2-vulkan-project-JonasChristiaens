#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>


constexpr uint32_t  WIDTH = 800;
constexpr uint32_t  HEIGHT = 600;

class HelloTriangleApplication {
public:
    void run() {
		initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    // =======================
    // Private class Functions
	// =======================

    void initWindow() {
        // Initialize GLFW library
		glfwInit();

		// Tell GLFW not to create an OpenGL context
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		// Disable window resizing  
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		// Create a window 
        // (width, height, name, optionally specify a monitor to open window on, only relevant to OpenGL)
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void initVulkan() {

    }

    void mainLoop() {
		// Loop until the user closes the window
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
		// Destroy the window
        glfwDestroyWindow(window);

		// Terminate GLFW library
        glfwTerminate();
    }


    // =======================
	// Private class variables
    // =======================

    GLFWwindow* window;
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}