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
#include "Renderpass.h"
#include "DescriptorSetLayout.h"
#include "Pipeline.h"
#include "CommandPool.h"
#include "Texture.h"
#include "Buffer.h"
#include "DescriptorPool.h"
#include "SceneManager.h"
#include "CommandBuffer.h"

#include <stdexcept>
#include <iostream>

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
    // =======================
    // Private class variables
    // =======================
    Window* m_pWindow = nullptr;

    Instance* m_pInstance = nullptr;
	Surface* m_pSurface = nullptr;
	Device* m_pDevice = nullptr;
    SwapChain* m_pSwapChain = nullptr;
    Renderpass* m_pRenderpass = nullptr;
	DescriptorSetLayout* m_pDescriptorSetLayout = nullptr;
	Pipeline* m_pPipeline = nullptr;
	CommandPool* m_pCommandPool = nullptr;
	Texture* m_pTexture = nullptr;

    std::vector<Vertex> m_vertices{};
    Buffer* m_pVertexBuffer = nullptr;
    std::vector<uint32_t> m_indices{};
    Buffer* m_pIndexBuffer = nullptr;;

    std::vector<Buffer*> m_pUniformBuffers{};
    std::vector<void*> m_UniformBuffersMapped{};

    DescriptorPool* m_pDescriptorPool = nullptr;
    CommandBuffer* m_pCommandBuffer = nullptr;
    SceneManager* m_pSceneManager = nullptr;

    // =======================
    // Private class Functions
	// =======================
    void initVulkan() 
    {
        m_pInstance = new Instance(); // createInstance & setupDebugMessenger
		m_pSurface = new Surface(m_pInstance, m_pWindow->getGLFWwindow()); // createSurface
		m_pDevice = new Device(m_pSurface, m_pInstance); // pickPhysicalDevice & createLogicalDevice
		m_pSwapChain = new SwapChain(m_pDevice, m_pWindow, m_pSurface); // createSwapChain & createImageViews
        m_pRenderpass = new Renderpass(m_pDevice, m_pSwapChain); // createRenderPass
		m_pDescriptorSetLayout = new DescriptorSetLayout(m_pDevice); // createDescriptorSetLayout
		m_pPipeline = new Pipeline(m_pDevice, m_pDescriptorSetLayout, m_pRenderpass); // createGraphicsPipeline
		m_pCommandPool = new CommandPool(m_pDevice); // createCommandPool
		m_pSwapChain->createResources(m_pRenderpass); // createColorResources, createDepthResources,createFramebuffers
		m_pTexture = new Texture(m_pDevice, m_pSwapChain, m_pCommandPool); // createTextureImage, createTextureImageView, createTextureSampler
        m_pSceneManager = new SceneManager(m_pDevice, m_pSwapChain, m_pRenderpass);
        m_pSceneManager->loadModel(m_vertices, m_indices); // loadModel

        // createVertexBuffer
        m_pVertexBuffer = new Buffer(m_pDevice, m_pCommandPool, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        m_pVertexBuffer->CreateStagingBuffer(sizeof(m_vertices[0]) * m_vertices.size(), m_vertices.data());

        // createIndexBuffer
        m_pIndexBuffer = new Buffer(m_pDevice, m_pCommandPool, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        m_pIndexBuffer->CreateStagingBuffer(sizeof(m_indices[0]) * m_indices.size(), m_indices.data());

        createUniformBuffers();

        m_pDescriptorPool = new DescriptorPool(m_pDevice, m_pTexture, m_pDescriptorSetLayout, m_pUniformBuffers); // createDescriptorPool & createDescriptorSets
        
        m_pCommandBuffer = new CommandBuffer(m_pDevice, m_pCommandPool, m_pRenderpass, m_pSwapChain, m_pPipeline, m_pVertexBuffer, m_pIndexBuffer, m_pDescriptorPool, m_pSceneManager);
        m_pCommandBuffer->createCommandBuffers(); //createCommandBuffers
        m_pSceneManager->createSyncObjects(); // createSyncObjects
    }

    void mainLoop() 
    {
		// Loop until the user closes the window
        while (!m_pWindow->shouldClose()) {
            m_pWindow->pollEvents();
            m_pSceneManager->drawFrame(m_pWindow, m_UniformBuffersMapped, m_pCommandBuffer, m_indices);
        }

        vkDeviceWaitIdle(m_pDevice->getDevice());
    }

    void cleanup() 
    {
        // cleanup Vulkan Memory
        m_pSwapChain->cleanupSwapChain();
        m_pTexture->cleanupTextures();

        for (size_t idx{}; idx < MAX_FRAMES_IN_FLIGHT; idx++) {
            m_pUniformBuffers[idx]->cleanupBuffer();
        }

        m_pDescriptorPool->cleanupDescriptorPool();
		m_pDescriptorSetLayout->cleanupDescriptorSetLayout();
        m_pIndexBuffer->cleanupBuffer();
        m_pVertexBuffer->cleanupBuffer();
        m_pPipeline->cleanupPipelines();
		m_pRenderpass->cleanupRenderPass();
        m_pSceneManager->cleanupScene();
        m_pCommandPool->destroyCommandPool();
        m_pDevice->cleanupDevice();
        m_pInstance->destroyDebugUtilsMessenger();
        m_pSurface->destroySurface();
        m_pInstance->destroyInstance();
        m_pWindow->cleanupWindow();

        // delete Pointers
        delete m_pSceneManager;
        for (size_t idx{}; idx < MAX_FRAMES_IN_FLIGHT; idx++) {
            delete m_pUniformBuffers[idx];
        }
        delete m_pDescriptorPool;
        delete m_pVertexBuffer;
        delete m_pIndexBuffer;
        delete m_pTexture;
		delete m_pCommandPool;
        delete m_pPipeline;
		delete m_pDescriptorSetLayout;
        delete m_pRenderpass;
        delete m_pSwapChain;
        delete m_pDevice;
		delete m_pSurface;
        delete m_pInstance;
        delete m_pWindow; 
    }

    bool hasStencilComponent(VkFormat format) 
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
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