#include "pch.h"
#include "utils.h"
#include "Commands.h"

// class including
#include "Window.h"
#include "Camera.h"
#include "Instance.h"
#include "Surface.h"
#include "Device.h"
#include "SwapChain.h"
#include "Image.h"
#include "Renderpass.h"
#include "DescriptorSetLayout.h"
#include "DescriptorSet.h"
#include "Pipeline.h"
#include "CommandPool.h"
#include "Texture.h"
#include "Buffer.h"
#include "DescriptorPool.h"
#include "SceneManager.h"
#include "CommandBuffer.h"

#include <stdexcept>
#include <iostream>
#include <SDL_events.h>

class HelloTriangleApplication {
public:
    void run() 
    {
       m_pTimer = new Timer();
       m_pWindow = new Window("Vulkan");
       //m_Camera = std::make_unique<Camera>(glm::vec3{ 0.0f, 0.0f, 0.0f }, 45.0f, aspectRatio, 0.1f, 100.0f);
       initVulkan();
       mainLoop();
       cleanup();
    }

private:
    // =======================
    // Private class variables
    // =======================
    Window* m_pWindow = nullptr;

    Timer* m_pTimer = nullptr;
    //std::unique_ptr<Camera> m_Camera;

    Instance* m_pInstance = nullptr;
	Surface* m_pSurface = nullptr;
	Device* m_pDevice = nullptr;
    SwapChain* m_pSwapChain = nullptr;
    Renderpass* m_pRenderpass = nullptr;

    DescriptorSet* m_pGlobalDescriptorSet = nullptr;
	DescriptorSetLayout* m_pGlobalDescriptorSetLayout = nullptr;

    std::vector<DescriptorSet*> m_pUboDescriptorSets{};
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
		m_pCommandPool = new CommandPool(m_pDevice); // createCommandPool
        m_pSwapChain->createResources(m_pRenderpass); // createColorResources, createDepthResources,createFramebuffers
        m_pSceneManager = new SceneManager(m_pDevice, m_pSwapChain, m_pRenderpass);
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
        size_t numTextures = m_pSceneManager->getTexturePaths().size();
        const uint32_t maxFramesInFlight = MAX_FRAMES_IN_FLIGHT;

        std::vector<VkDescriptorPoolSize> poolSizes = {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxFramesInFlight },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(numTextures) }
        };
        uint32_t maxSets = 1 + maxFramesInFlight;

        m_pDescriptorPool = new DescriptorPool(m_pDevice, poolSizes, maxSets); // createDescriptorPool
        createDescriptorSets();

		m_pPipeline = new Pipeline(m_pDevice, m_pGlobalDescriptorSetLayout, m_pUboDescriptorSetLayout, m_pRenderpass); // createGraphicsPipeline
        m_pCommandBuffer = new CommandBuffer(m_pDevice, m_pCommandPool, m_pRenderpass, m_pSwapChain, m_pPipeline, m_pVertexBuffers, m_pIndexBuffers, m_pSceneManager);
        m_pCommandBuffer->createCommandBuffers(); //createCommandBuffers
        m_pSceneManager->createSyncObjects(); // createSyncObjects
    }

    void mainLoop() 
    {
       // Loop until the user closes the window
       while (!m_pWindow->shouldClose()) {
           m_pWindow->pollEvents();
           //m_Camera->Update(m_pTimer);
           m_pSceneManager->drawFrame(m_pWindow, m_UniformBuffersMapped, m_pCommandBuffer, m_pGlobalDescriptorSet, m_pUboDescriptorSets);
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
        m_pGlobalDescriptorSetLayout->cleanupDescriptorSetLayout();
        m_pUboDescriptorSetLayout->cleanupDescriptorSetLayout();

        for (size_t idx{}; idx < MAX_FRAMES_IN_FLIGHT; idx++) {
            m_pIndexBuffers[idx]->cleanupBuffer();
        }
        for (size_t idx{}; idx < MAX_FRAMES_IN_FLIGHT; idx++) {
            m_pVertexBuffers[idx]->cleanupBuffer();
        }

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

        for (size_t idx{}; idx < MAX_FRAMES_IN_FLIGHT; idx++) {
            delete m_pVertexBuffers[idx];
        }
        for (size_t idx{}; idx < MAX_FRAMES_IN_FLIGHT; idx++) {
            delete m_pIndexBuffers[idx];
        }

        delete m_pTexture;
		delete m_pCommandPool;
        delete m_pPipeline;
		delete m_pGlobalDescriptorSet;
		delete m_pUboDescriptorSetLayout;
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

    void createDescriptorSets()
    {
        // Create Global Descriptor Set
        m_pGlobalDescriptorSetLayout = new DescriptorSetLayout(m_pDevice);
		m_pGlobalDescriptorSetLayout->createGlobalDescriptorSetLayout(m_pSceneManager->getTexturePaths().size());

		m_pGlobalDescriptorSet = new DescriptorSet(m_pDevice, m_pTexture, m_pGlobalDescriptorSetLayout, m_pDescriptorPool); 
        m_pGlobalDescriptorSet->createDescriptorSets();
		m_pGlobalDescriptorSet->updateGlobalDescriptorSets(m_pSceneManager->getTexturePaths().size()); 

        // Create Ubo Descriptor Set
        m_pUboDescriptorSetLayout = new DescriptorSetLayout(m_pDevice);
		m_pUboDescriptorSetLayout->createUboDescriptorSetLayout();

        for (int idx{}; idx < MAX_FRAMES_IN_FLIGHT; idx++)
        {
            m_pUboDescriptorSets.push_back(new DescriptorSet(m_pDevice, m_pTexture, m_pUboDescriptorSetLayout, m_pDescriptorPool)); 
            m_pUboDescriptorSets[idx]->createDescriptorSets(); 

            m_pUboDescriptorSets[idx]->updateUboDescriptorSets(m_pUniformBuffers);
        }
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