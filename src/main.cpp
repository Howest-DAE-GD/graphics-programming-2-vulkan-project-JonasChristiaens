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

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <chrono>
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

    std::vector<VkBuffer> uniformBuffers{};
    std::vector<VkDeviceMemory> uniformBuffersMemory{};
    std::vector<void*> uniformBuffersMapped{};

    VkDescriptorPool descriptorPool{};
    std::vector<VkDescriptorSet> descriptorSets{};

    std::vector<VkCommandBuffer> commandBuffers{}; // niet in buffer, werkt seperately

    std::vector<VkSemaphore> imageAvailableSemaphores{};
    std::vector<VkSemaphore> renderFinishedSemaphores{};
    std::vector<VkFence> inFlightFences{};
    uint32_t currentFrame{};

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

        loadModel();

        // createVertexBuffer
        m_pVertexBuffer = new Buffer(m_pDevice, m_pCommandPool, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        m_pVertexBuffer->CreateStagingBuffer(sizeof(vertices[0]) * vertices.size(), vertices.data()); 

        // createIndexBuffer
        m_pIndexBuffer = new Buffer(m_pDevice, m_pCommandPool, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        m_pIndexBuffer->CreateStagingBuffer(sizeof(indices[0]) * indices.size(), indices.data()); 

        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffers();
        createSyncObjects();
    }

    void mainLoop() 
    {
		// Loop until the user closes the window
        while (!m_pWindow->shouldClose()) {
            m_pWindow->pollEvents();
            drawFrame();
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
            vkDestroyBuffer(m_pDevice->getDevice(), uniformBuffers[idx], nullptr);
            vkFreeMemory(m_pDevice->getDevice(), uniformBuffersMemory[idx], nullptr);
        }

        vkDestroyDescriptorPool(m_pDevice->getDevice(), descriptorPool, nullptr);
		m_pDescriptorSetLayout->cleanupDescriptorSetLayout();
        m_pIndexBuffer->cleanupBuffer();
        m_pVertexBuffer->cleanupBuffer();
        m_pPipeline->cleanupPipelines();
		m_pRenderpass->cleanupRenderPass();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(m_pDevice->getDevice(), renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(m_pDevice->getDevice(), imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(m_pDevice->getDevice(), inFlightFences[i], nullptr);
        }

        m_pCommandPool->destroyCommandPool();
        m_pDevice->cleanupDevice();
        m_pInstance->destroyDebugUtilsMessenger();
        m_pSurface->destroySurface();
        m_pInstance->destroyInstance();
        m_pWindow->cleanupWindow();

        // delete Pointers
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

    void loadModel()
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
            throw std::runtime_error(warn + err);
        }

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};

                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.color = { 1.0f, 1.0f, 1.0f };

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }

    // Function that allocates the buffers
    void createUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t idx{}; idx < MAX_FRAMES_IN_FLIGHT; idx++)
        {
            Buffer::createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[idx], uniformBuffersMemory[idx], m_pDevice);

            // mapping of buffer to get a pointer to which we can write data
            // => persistent mapping
            vkMapMemory(m_pDevice->getDevice(), uniformBuffersMemory[idx], 0, bufferSize, 0, &uniformBuffersMapped[idx]);
        }
    }

	// Function to create pool for descriptor sets
    void createDescriptorPool()
    {
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        // pool size structure
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT); // max nr of descriptor sets that may be allocated

        if (vkCreateDescriptorPool(m_pDevice->getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        {
			throw std::runtime_error("failed to create descriptor pool!");
        }
    }

	// Function to allocate descriptor sets
    void createDescriptorSets()
    {
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_pDescriptorSetLayout->getDescriptorSetLayout());

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool; // Specify pool to allocate from
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT); // Number of sets to allocate
		allocInfo.pSetLayouts = layouts.data(); // Layout to base them on

		// vector that holds the descriptor sets
        descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(m_pDevice->getDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t idx{}; idx < MAX_FRAMES_IN_FLIGHT; idx++) 
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[idx]; // specify buffer to bind
            // specify region within that contains data for descriptor
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = m_pTexture->getTextureImageView();
            imageInfo.sampler = m_pTexture->getTextureSampler();

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[idx]; // specify descriptor set to update
            descriptorWrites[0].dstBinding = 0; // specify binding within the set
            descriptorWrites[0].dstArrayElement = 0; // specify array element within binding
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // specify type of descriptor
            descriptorWrites[0].descriptorCount = 1; // specify number of descriptors to update
            descriptorWrites[0].pBufferInfo = &bufferInfo; // specify array of descriptors

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[idx];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            // update descriptor set
            vkUpdateDescriptorSets(m_pDevice->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
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

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pPipeline->getPipelineLayout(), 0, 1, &descriptorSets[currentFrame], 0, nullptr);
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void createSyncObjects()
    {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(m_pDevice->getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_pDevice->getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_pDevice->getDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }

    }

    // Generate a new transformation every frame to make geometry spin around
    void updateUniformBuffer(uint32_t currentImage)
    {
        // Calculate time in seconds
        static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        
        // Define model, view and projection transformations in UBO
        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), m_pSwapChain->getExtent().width / (float)m_pSwapChain->getExtent().height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        // Copy data in UBO to current uniform buffer (! without staging buffer)
        memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

    void drawFrame()
    {
        vkWaitForFences(m_pDevice->getDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_pDevice->getDevice(), m_pSwapChain->getSwapChain(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            m_pSwapChain->recreateSwapChain(m_pRenderpass);
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        updateUniformBuffer(currentFrame);

        vkResetFences(m_pDevice->getDevice(), 1, &inFlightFences[currentFrame]);

        vkResetCommandBuffer(commandBuffers[currentFrame], 0);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(m_pDevice->getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { m_pSwapChain->getSwapChain()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(m_pDevice->getPresentQueue(), &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_pWindow->wasWindowResized()) {
            m_pWindow->resetWindowResizedFlag();
            m_pSwapChain->recreateSwapChain(m_pRenderpass);
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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