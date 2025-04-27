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
#include <stdexcept>

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
    Window* m_pWindow{};

    Instance* m_pInstance{};
	Surface* m_pSurface{};

	Device* m_pDevice{};

    SwapChain* m_pSwapChain{};

	Renderpass* m_pRenderpass{}; 
	DescriptorSetLayout* m_pDescriptorSetLayout{};
	Pipeline* m_pPipeline{};

	CommandPool* m_pCommandPool{};

	Texture* m_pTexture{};

    std::vector<Vertex> vertices{};
    Buffer* m_pVertexBuffer{};
    std::vector<uint32_t> indices{};
    Buffer* m_pIndexBuffer{};

    std::vector<Buffer*> m_pUniformBuffers{};
    std::vector<void*> m_UniformBuffersMapped{};

    DescriptorPool* m_pDescriptor{};

    std::vector<VkCommandBuffer> commandBuffers{}; // niet in buffer, werkt seperately

    SceneManager* m_pSceneManager{};

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
        m_pSceneManager->loadModel(vertices, indices); // loadModel

        // createVertexBuffer
        m_pVertexBuffer = new Buffer(m_pDevice, m_pCommandPool, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        m_pVertexBuffer->CreateStagingBuffer(sizeof(vertices[0]) * vertices.size(), vertices.data()); 

        // createIndexBuffer
        m_pIndexBuffer = new Buffer(m_pDevice, m_pCommandPool, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        m_pIndexBuffer->CreateStagingBuffer(sizeof(indices[0]) * indices.size(), indices.data()); 

        createUniformBuffers();

        m_pDescriptor = new DescriptorPool(m_pDevice, m_pTexture, m_pDescriptorSetLayout, m_pUniformBuffers); // createDescriptorPool & createDescriptorSets

        createCommandBuffers();
        m_pSceneManager->createSyncObjects(); // createSyncObjects
    }

    void mainLoop() 
    {
		// Loop until the user closes the window
        while (!m_pWindow->shouldClose()) {
            m_pWindow->pollEvents();
            m_pSceneManager->drawFrame(m_pWindow, m_UniformBuffersMapped);
        }

        vkDeviceWaitIdle(m_pDevice->getDevice());
    }

    void cleanup() 
    {
        // cleanup Vulkan Memory
        m_pSwapChain->cleanupSwapChain();
        m_pTexture->cleanupTextures();

        for (size_t idx{}; idx < MAX_FRAMES_IN_FLIGHT; idx++)
        {
            m_pUniformBuffers[idx]->cleanupBuffer();
        }

        m_pDescriptor->cleanupDescriptorPool();
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
        for (size_t idx{}; idx < MAX_FRAMES_IN_FLIGHT; idx++)
        {
            delete m_pUniformBuffers[idx];
        }
        delete m_pDescriptor;
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


        // -> deletionqueue is best for this, 
    }

    bool hasStencilComponent(VkFormat format) 
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    VkSampleCountFlagBits getMaxUsableSampleCount() {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(m_pDevice->getPhysicalDevice(), &physicalDeviceProperties);

        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
        if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
        if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
        if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
        if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
        if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

        return VK_SAMPLE_COUNT_1_BIT;
    }

    // Function that allocates the buffers
    void createUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        m_pUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
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

    void createCommandBuffers()
    {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_pCommandPool->getCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

        if (vkAllocateCommandBuffers(m_pDevice->getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) 
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) 
            throw std::runtime_error("failed to begin recording command buffer!");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_pRenderpass->getRenderPass();
        renderPassInfo.framebuffer = m_pSwapChain->getFramebuffers()[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_pSwapChain->getExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pPipeline->getGraphicsPipeline());

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(m_pSwapChain->getExtent().width);
            viewport.height = static_cast<float>(m_pSwapChain->getExtent().height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = { 0, 0 };
            scissor.extent = m_pSwapChain->getExtent();
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            VkBuffer vertexBuffers[] = { m_pVertexBuffer->getBuffer() };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffer, m_pIndexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pPipeline->getPipelineLayout(), 0, 1, &m_pDescriptor->getDescriptorSets()[m_pSceneManager->getCurrentFrame()], 0, nullptr);
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
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