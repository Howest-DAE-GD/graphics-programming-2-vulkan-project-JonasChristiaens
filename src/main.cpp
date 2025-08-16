#include "pch.h"
#include "utils.h"
#include "Commands.h"

// class including
#include "Window.h"
#include "Instance.h"
#include "Surface.h"
#include "Device.h"
#include "SwapChain.h"
#include "Image.h"
#include "DescriptorSetLayout.h"
#include "DescriptorSet.h"
#include "Pipeline.h"
#include "CommandPool.h"
#include "Texture.h"
#include "Buffer.h"
#include "DescriptorPool.h"
#include "SceneManager.h"
#include "CommandBuffer.h"
#include "camera.h"

#include <stdexcept>
#include <iostream>
#include <SDL_timer.h>
#include "glm/ext.hpp"

class HelloTriangleApplication {
public:
    void run() 
    {
       m_pWindow = new Window("Vulkan");
       initVulkan();

       mainLoop();
       cleanup();
    }

private:
    // Private class variables
    Window* m_pWindow = nullptr;

    Camera* m_pCamera = nullptr;
    double lastX = WIDTH / 2.0;
    double lastY = HEIGHT / 2.0;
    bool firstMouse = true;

    Instance* m_pInstance = nullptr;

	Surface* m_pSurface = nullptr;

	Device* m_pDevice = nullptr;

    SwapChain* m_pSwapChain = nullptr;

    DescriptorSet* m_pGlobalDescriptorSet = nullptr;
    DescriptorSetLayout* m_pGlobalDescriptorSetLayout = nullptr;

    DescriptorSet* m_pUboDescriptorSet = nullptr;
    DescriptorSetLayout* m_pUboDescriptorSetLayout = nullptr;

	Pipeline* m_pPipeline = nullptr;

	CommandPool* m_pCommandPool = nullptr;

	Texture* m_pTexture = nullptr;

	std::vector<Buffer*> m_pVertexBuffers{};
    std::vector<Buffer*> m_pIndexBuffers{};

    std::vector<Buffer*> m_pUniformBuffers{};
    std::vector<void*> m_UniformBuffersMapped{};

    DescriptorPool* m_pDescriptorPool = nullptr;

    CommandBuffer* m_pCommandBuffer = nullptr;

    SceneManager* m_pSceneManager = nullptr;

    // Private class Functions
    void initVulkan() 
    {
        m_pInstance = new Instance(); // createInstance & setupDebugMessenger
		m_pSurface = new Surface(m_pInstance, m_pWindow->getGLFWwindow()); // createSurface
		m_pDevice = new Device(m_pSurface, m_pInstance); // pickPhysicalDevice & createLogicalDevice
		m_pCommandPool = new CommandPool(m_pDevice); // createCommandPool
		m_pSwapChain = new SwapChain(m_pDevice, m_pWindow, m_pSurface, m_pCommandPool); // createSwapChain & createImageViews

        m_pSwapChain->createResources(); // createColorResources, createDepthResources,createFramebuffers

        m_pSceneManager = new SceneManager(m_pDevice, m_pSwapChain);
        m_pSceneManager->loadModel(); // loadModel
		m_pTexture = new Texture(m_pDevice, m_pSwapChain, m_pCommandPool, m_pSceneManager); // createTextureImage, createTextureImageView, createTextureSampler

        // for each separate mesh, create separate buffers (TODO: change to use single large buffer)
        for (int i{}; i < m_pSceneManager->getMeshes().size(); i++) {

            // createVertexBuffer
            m_pVertexBuffers.push_back(new Buffer(m_pDevice, m_pCommandPool, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
            m_pVertexBuffers[i]->CreateStagingBuffer(sizeof(m_pSceneManager->getMeshes()[i].vertices[0]) * m_pSceneManager->getMeshes()[i].vertices.size(), m_pSceneManager->getMeshes()[i].vertices.data());

            // createIndexBuffer
            m_pIndexBuffers.push_back(new Buffer(m_pDevice, m_pCommandPool, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT));
            m_pIndexBuffers[i]->CreateStagingBuffer(sizeof(m_pSceneManager->getMeshes()[i].indices[0]) * m_pSceneManager->getMeshes()[i].indices.size(), m_pSceneManager->getMeshes()[i].indices.data());
        }

        createUniformBuffers();
        
        // Calculate descriptor pool requirements
        size_t numTextures = m_pSceneManager->getAlbedoPaths().size();
        const uint32_t maxFramesInFlight = MAX_FRAMES_IN_FLIGHT;

        std::vector<VkDescriptorPoolSize> poolSizes = {
            // Uniform buffers (for UBO sets)
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxFramesInFlight },

            // Global set requirements:
            // 1 shared sampler (binding 0)
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1 },

            // Texture array (binding 1) + G-buffer textures (bindings 2-5)
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, static_cast<uint32_t>(numTextures * 2 + 4) }
        };
        uint32_t maxSets = 1 + maxFramesInFlight;

        m_pDescriptorPool = new DescriptorPool(m_pDevice, poolSizes, maxSets); // createDescriptorPool
        createDescriptorSets();

		m_pPipeline = new Pipeline(m_pDevice, m_pGlobalDescriptorSetLayout, m_pUboDescriptorSetLayout, m_pSwapChain); // createGraphicsPipeline, createDepthPrepassPipeline, createGeometryPipeline & createLightingPipeline
        m_pCommandBuffer = new CommandBuffer(m_pDevice, m_pCommandPool, m_pSwapChain, m_pPipeline, m_pVertexBuffers, m_pIndexBuffers, m_pSceneManager);
        m_pCommandBuffer->createCommandBuffers(); //createCommandBuffers
        m_pSceneManager->createSyncObjects(); // createSyncObjects
    }

    void mainLoop() 
    {
	   // Create camera with initial position and orientation
       float aspectRatio = static_cast<float>(WIDTH) / HEIGHT;
       glm::vec3 camPos = glm::vec3(0.0f, 1.5f, 0.0f);
       float yaw = 0.0f;   // Looking down +Z
       float pitch = 0.0f; // Level

       m_pCamera = new Camera(camPos, yaw, pitch, 90.0f, aspectRatio, 0.1f, 100.0f);

       glfwSetWindowUserPointer(m_pWindow->getGLFWwindow(), this);
       glfwSetCursorPosCallback(m_pWindow->getGLFWwindow(), mouse_callback);
       glfwSetInputMode(m_pWindow->getGLFWwindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

       float lastFrameTime = glfwGetTime();

       // Loop until the user closes the window
       while (!m_pWindow->shouldClose()) {
           float currentFrameTime = glfwGetTime();
           float deltaTime = currentFrameTime - lastFrameTime;
           lastFrameTime = currentFrameTime;

           m_pWindow->pollEvents();

           GLFWwindow* window = m_pWindow->getGLFWwindow();
           if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) m_pCamera->processKeyboard(GLFW_KEY_W, deltaTime);
           if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) m_pCamera->processKeyboard(GLFW_KEY_S, deltaTime);
           if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) m_pCamera->processKeyboard(GLFW_KEY_A, deltaTime);
           if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) m_pCamera->processKeyboard(GLFW_KEY_D, deltaTime);
           if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) m_pCamera->processKeyboard(GLFW_KEY_SPACE, deltaTime);
           if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) m_pCamera->processKeyboard(GLFW_KEY_LEFT_CONTROL, deltaTime);

           m_pSceneManager->drawFrame(m_pWindow, m_UniformBuffersMapped, m_pCommandBuffer, m_pGlobalDescriptorSet, m_pUboDescriptorSet, m_pGlobalDescriptorSetLayout, m_pUboDescriptorSetLayout, m_pUniformBuffers, m_pPipeline, m_pCamera);
           
		   //std::cout << m_pCamera->getPosition() << std::endl;
       }

       vkDeviceWaitIdle(m_pDevice->getDevice());
    }

    void cleanup() 
    {
        // --- Destroy Vulkan resources ---
        // 1. SceneManager: destroy semaphores/fences, etc.
        if (m_pSceneManager) m_pSceneManager->cleanupScene();

        // 2. Texture: destroy images, image views, samplers, device memory
        if (m_pTexture) m_pTexture->cleanupTextures();

        // 3. Buffers: destroy all vertex/index/uniform buffers
        if (!m_pVertexBuffers.empty()) {
            for (size_t i = 0; i < m_pVertexBuffers.size(); ++i) {
                if (m_pVertexBuffers[i]) m_pVertexBuffers[i]->cleanupBuffer();
            }
        }
        if (!m_pIndexBuffers.empty()) {
            for (size_t i = 0; i < m_pIndexBuffers.size(); ++i) {
                if (m_pIndexBuffers[i]) m_pIndexBuffers[i]->cleanupBuffer();
            }
        }
        if (!m_pUniformBuffers.empty()) {
            for (size_t i = 0; i < m_pUniformBuffers.size(); ++i) {
                if (m_pUniformBuffers[i]) m_pUniformBuffers[i]->cleanupBuffer();
            }
        }

        // 4. Descriptor sets, pools, layouts
        if (m_pGlobalDescriptorSet) m_pGlobalDescriptorSet->cleanupDescriptorSet();
        if (m_pDescriptorPool) m_pDescriptorPool->cleanupDescriptorPool();
        if (m_pGlobalDescriptorSetLayout) m_pGlobalDescriptorSetLayout->cleanupDescriptorSetLayout();
        if (m_pUboDescriptorSetLayout) m_pUboDescriptorSetLayout->cleanupDescriptorSetLayout();

        // 5. Pipelines
        if (m_pPipeline) m_pPipeline->cleanupPipelines();

        // 6. SwapChain (includes G-buffer images, views, swapchain images/views)
        if (m_pSwapChain) m_pSwapChain->cleanupSwapChain();

        // 7. Command pool
        if (m_pCommandPool) m_pCommandPool->destroyCommandPool();

        // 8. Device 
        if (m_pDevice) m_pDevice->cleanupDevice();

        // 9. Instance-level resources (debug messenger, surface, instance)
        if (m_pSurface) m_pSurface->destroySurface();
        if (m_pInstance) m_pInstance->destroyDebugUtilsMessenger();
        if (m_pInstance) m_pInstance->destroyInstance();

        // 10. Window system resources
        if (m_pWindow) m_pWindow->cleanupWindow();


        // --- Delete pointers ---
        // Buffers
        for (size_t i = 0; i < m_pVertexBuffers.size(); ++i) { delete m_pVertexBuffers[i]; }
        m_pVertexBuffers.clear();
        for (size_t i = 0; i < m_pIndexBuffers.size(); ++i) { delete m_pIndexBuffers[i]; }
        m_pIndexBuffers.clear();
        for (size_t i = 0; i < m_pUniformBuffers.size(); ++i) { delete m_pUniformBuffers[i]; }
        m_pUniformBuffers.clear();

        // Descriptor sets
        delete m_pGlobalDescriptorSet;
        delete m_pDescriptorPool;
        delete m_pGlobalDescriptorSetLayout;
        delete m_pUboDescriptorSetLayout;

        // Other objects
        delete m_pSceneManager;
        delete m_pTexture;
        delete m_pCommandPool;
        delete m_pPipeline;
        delete m_pSwapChain;
        delete m_pDevice;
        delete m_pSurface;
        delete m_pInstance;
        delete m_pWindow;

        delete m_pCamera;
    }

    bool hasStencilComponent(VkFormat format) 
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    void createDescriptorSets()
    {
        // Create Global Descriptor Set (with G-buffer bindings)
        m_pGlobalDescriptorSetLayout = new DescriptorSetLayout(m_pDevice);
		m_pGlobalDescriptorSetLayout->createGlobalDescriptorSetLayout(m_pSceneManager->getAlbedoPaths().size(), true);

		m_pGlobalDescriptorSet = new DescriptorSet(m_pDevice, m_pTexture, m_pGlobalDescriptorSetLayout, m_pDescriptorPool, 1); 
        m_pGlobalDescriptorSet->createDescriptorSets();
		m_pGlobalDescriptorSet->updateGlobalDescriptorSets(m_pSceneManager->getAlbedoPaths().size()); 

        // Update with G-buffer textures after they're created
        const GBuffer& gBuffer = m_pSwapChain->getGBuffer();
        m_pGlobalDescriptorSet->updateGBufferDescriptorSets(
            gBuffer.views[0], // Position
            gBuffer.views[1], // Normal
            gBuffer.views[2], // Albedo
            gBuffer.views[3]  // Material
        );

        // Create Ubo Descriptor Set
        m_pUboDescriptorSetLayout = new DescriptorSetLayout(m_pDevice);
        m_pUboDescriptorSetLayout->createUboDescriptorSetLayout();

        m_pUboDescriptorSet = new DescriptorSet(m_pDevice, m_pTexture, m_pUboDescriptorSetLayout, m_pDescriptorPool, MAX_FRAMES_IN_FLIGHT);
        m_pUboDescriptorSet->createDescriptorSets();
        m_pUboDescriptorSet->updateUboDescriptorSets(m_pUniformBuffers);
    }

    // Function that allocates the buffers
    void createUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        m_UniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t idx{}; idx < MAX_FRAMES_IN_FLIGHT; idx++)
        {
            m_pUniformBuffers.emplace_back(new Buffer(m_pDevice, m_pCommandPool, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT));
            m_pUniformBuffers[idx]->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_pUniformBuffers[idx]->getBuffer(), m_pUniformBuffers[idx]->getBufferMemory(), m_pDevice);

            // mapping of buffer to get a pointer to which we can write data
            // => persistent mapping
            vkMapMemory(m_pDevice->getDevice(), m_pUniformBuffers[idx]->getBufferMemory(), 0, bufferSize, 0, &m_UniformBuffersMapped[idx]);
        }
    }

    // Mouse callback
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
        HelloTriangleApplication* app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
        if (app->firstMouse) {
            app->lastX = xpos;
            app->lastY = ypos;
            app->firstMouse = false;
        }
        float xoffset = xpos - app->lastX;
        float yoffset = app->lastY - ypos; // reversed: y ranges bottom to top
        app->lastX = xpos;
        app->lastY = ypos;
        app->m_pCamera->processMouse(xoffset, yoffset);
    }
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