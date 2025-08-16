#include "pch.h"
#include "utils.h"
#include "CommandBuffer.h"

#include "Device.h"
#include "CommandPool.h"
#include "SwapChain.h"
#include "Pipeline.h"
#include "Buffer.h"
#include "SceneManager.h"
#include "DescriptorSet.h"
#include <stdexcept>
#include <iostream>

CommandBuffer::CommandBuffer(Device* device, CommandPool* commandPool, SwapChain* swapChain, Pipeline* pipeline, std::vector<Buffer*> vertexBuffer,
    std::vector<Buffer*> indexBuffer, SceneManager* sceneManager)
	: m_pDevice(device)
	, m_pCommandPool(commandPool)
    , m_pSwapChain(swapChain)
    , m_pPipeline(pipeline)
    , m_pVertexBuffers(vertexBuffer)
    , m_pIndexBuffers(indexBuffer)
    , m_pSceneManager(sceneManager)
{
}

CommandBuffer::~CommandBuffer()
{
    if (!m_CommandBuffers.empty()) {
        vkFreeCommandBuffers(m_pDevice->getDevice(), m_pCommandPool->getCommandPool(),
            static_cast<uint32_t>(m_CommandBuffers.size()), m_CommandBuffers.data());
        m_CommandBuffers.clear();
    }
}

void CommandBuffer::createCommandBuffers()
{
	m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_pCommandPool->getCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

	if (vkAllocateCommandBuffers(m_pDevice->getDevice(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

void CommandBuffer::recordDeferredCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, std::vector<Mesh> Meshes, DescriptorSet* globalDescriptorSet, std::vector<DescriptorSet*> uboDescriptorSets)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("failed to begin recording command buffer!");

    VkClearValue depthClearValue = { .depthStencil = { 1.0f, 0 } };

    // Initial layout transitions
    {
        std::array<VkImageMemoryBarrier, 2> barriers{};

        // Swapchain image
        barriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barriers[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barriers[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barriers[0].newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barriers[0].image = m_pSwapChain->getSwapChainImages()[imageIndex];
        barriers[0].subresourceRange = {
            VK_IMAGE_ASPECT_COLOR_BIT,
            0, 1, 0, 1
        };

        // Depth image
        barriers[1].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barriers[1].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        barriers[1].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barriers[1].newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        barriers[1].image = m_pSwapChain->m_pImage->getDepthImage();
        barriers[1].subresourceRange = {
            VK_IMAGE_ASPECT_DEPTH_BIT,
            0, 1, 0, 1
        };

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            0,
            0, nullptr,
            0, nullptr,
            static_cast<uint32_t>(barriers.size()), barriers.data()
        );
    }

    // ---- G-buffer layout transition (for geometry pass) ----
    {
        const GBuffer& gBuffer = m_pSwapChain->getGBuffer();
        VkImage gBufferImages[] = { gBuffer.position, gBuffer.normal, gBuffer.albedo, gBuffer.material };
        std::array<VkImageMemoryBarrier, 4> barriers{};

        for (int i = 0; i < 4; ++i) {
            barriers[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barriers[i].oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // Previous frame's usage
            barriers[i].newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barriers[i].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barriers[i].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barriers[i].image = gBufferImages[i];
            barriers[i].subresourceRange = {
                VK_IMAGE_ASPECT_COLOR_BIT,
                0, 1, 0, 1
            };
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, // srcStageMask
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // dstStageMask
            0,
            0, nullptr,
            0, nullptr,
            static_cast<uint32_t>(barriers.size()), barriers.data()
        );
    }

    // ---- Depth Prepass ----
    {
        VkRenderingAttachmentInfoKHR depthAttachment = {};
        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        depthAttachment.imageView = m_pSwapChain->m_pImage->getDepthImageView();
        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.clearValue = depthClearValue;

        VkRenderingInfoKHR renderingInfo = {};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
        renderingInfo.renderArea = { {0, 0}, m_pSwapChain->getExtent() };
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 0;
        renderingInfo.pDepthAttachment = &depthAttachment;

        vkCmdBeginRendering(commandBuffer, &renderingInfo);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pPipeline->getDepthPrepassPipeline());

        // Set dynamic viewport/scissor
        VkViewport viewport{};
        viewport.width = static_cast<float>(m_pSwapChain->getExtent().width);
        viewport.height = static_cast<float>(m_pSwapChain->getExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{ {0, 0}, m_pSwapChain->getExtent() };
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        // Draw all meshes
        for (int i = 0; i < m_pIndexBuffers.size(); ++i) {
            VkBuffer vertexBuffers[] = { m_pVertexBuffers[i]->getBuffer() };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffer, m_pIndexBuffers[i]->getBuffer(), 0, VK_INDEX_TYPE_UINT32);

            // Bind descriptor sets
            VkDescriptorSet descriptorSets[] = {
                globalDescriptorSet->getDescriptorSets(),
                uboDescriptorSets[m_pSceneManager->getCurrentFrame()]->getDescriptorSets()
            };
            vkCmdBindDescriptorSets(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_pPipeline->getDepthPrepassLayout(),
                0, 2, descriptorSets, 0, nullptr
            );

            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(Meshes[i].indices.size()), 1, 0, 0, 0);
        }

        vkCmdEndRendering(commandBuffer);
    }

    // Transition depth for reading
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
        barrier.image = m_pSwapChain->m_pImage->getDepthImage();
        barrier.subresourceRange = {
            VK_IMAGE_ASPECT_DEPTH_BIT,
            0, 1, 0, 1
        };

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }

    // ---- Geometry Pass ----
    recordGeometryPass(commandBuffer, Meshes, globalDescriptorSet, uboDescriptorSets[m_pSceneManager->getCurrentFrame()]);

    // Transition G-buffer textures for reading (for lighting pass)
    {
        std::array<VkImageMemoryBarrier, 4> barriers{};
        const GBuffer& gBuffer = m_pSwapChain->getGBuffer();
        VkImage gBufferImages[] = { gBuffer.position, gBuffer.normal, gBuffer.albedo, gBuffer.material };

        for (int i = 0; i < 4; i++) {
            barriers[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barriers[i].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barriers[i].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barriers[i].oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barriers[i].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barriers[i].image = gBufferImages[i];
            barriers[i].subresourceRange = {
                VK_IMAGE_ASPECT_COLOR_BIT,
                0, 1, 0, 1
            };
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            static_cast<uint32_t>(barriers.size()), barriers.data()
        );
    }

    // ---- Lighting Pass ----
    recordLightingPass(commandBuffer, imageIndex, globalDescriptorSet);

    // Transition swapchain image for presentation
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = 0;
        barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrier.image = m_pSwapChain->getSwapChainImages()[imageIndex];
        barrier.subresourceRange = {
            VK_IMAGE_ASPECT_COLOR_BIT,
            0, 1, 0, 1
        };

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void CommandBuffer::recordGeometryPass(VkCommandBuffer commandBuffer, const std::vector<Mesh>& Meshes, DescriptorSet* globalDescriptorSet, DescriptorSet* uboDescriptorSet)
{
    const GBuffer& gBuffer = m_pSwapChain->getGBuffer();

    // Setup attachments
    std::array<VkRenderingAttachmentInfo, 4> colorAttachments{};
    VkImageView gBufferViews[] = {
        gBuffer.views[0], // Position
        gBuffer.views[1], // Normal
        gBuffer.views[2], // Albedo
        gBuffer.views[3]  // Material
    };

    VkClearValue clearValues[5] = {};
    clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} }; // Position
    clearValues[1].color = { {0.0f, 0.0f, 0.0f, 1.0f} }; // Normal
    clearValues[2].color = { {0.0f, 0.0f, 0.0f, 1.0f} }; // Albedo
    clearValues[3].color = { {0.0f, 0.0f, 0.0f, 1.0f} }; // Material
    clearValues[4].depthStencil = { 1.0f, 0 };           // Depth

    for (int i = 0; i < 4; i++) {
        colorAttachments[i].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachments[i].imageView = gBufferViews[i];
        colorAttachments[i].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachments[i].clearValue = clearValues[i];
    }

    VkRenderingAttachmentInfo depthAttachment{};
    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachment.imageView = m_pSwapChain->m_pImage->getDepthImageView();
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.clearValue = clearValues[4];

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea = { {0, 0}, m_pSwapChain->getExtent() };
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
    renderingInfo.pColorAttachments = colorAttachments.data();
    renderingInfo.pDepthAttachment = &depthAttachment;

    vkCmdBeginRendering(commandBuffer, &renderingInfo);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_pPipeline->getGeometryPipeline());

    // Set dynamic viewport/scissor
    VkViewport viewport{};
    viewport.width = static_cast<float>(m_pSwapChain->getExtent().width);
    viewport.height = static_cast<float>(m_pSwapChain->getExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{ {0, 0}, m_pSwapChain->getExtent() };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    // Draw all meshes
    for (int i = 0; i < m_pIndexBuffers.size(); ++i) {
        VkBuffer vertexBuffers[] = { m_pVertexBuffers[i]->getBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, m_pIndexBuffers[i]->getBuffer(), 0,
            VK_INDEX_TYPE_UINT32);

        // Bind descriptor sets
        VkDescriptorSet descriptorSets[] = {
            globalDescriptorSet->getDescriptorSets(),
            uboDescriptorSet->getDescriptorSets()
        };
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pPipeline->getPipelineLayout(),
            0, 2, descriptorSets, 0, nullptr);

        // Push material index
        vkCmdPushConstants(
            commandBuffer,
            m_pPipeline->getPipelineLayout(),
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0, sizeof(uint32_t),
            &Meshes[i].materialIndex
        );

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(Meshes[i].indices.size()),
            1, 0, 0, 0);
    }

    vkCmdEndRendering(commandBuffer);
}

void CommandBuffer::recordLightingPass(VkCommandBuffer commandBuffer, uint32_t imageIndex, DescriptorSet* globalDescriptorSet)
{
    // Setup final color attachment
    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView = m_pSwapChain->getImageViews()[imageIndex];
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea = { {0, 0}, m_pSwapChain->getExtent() };
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;
    renderingInfo.pDepthAttachment = nullptr;

    vkCmdBeginRendering(commandBuffer, &renderingInfo);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_pPipeline->getLightingPipeline());

    // Set dynamic viewport/scissor
    VkViewport viewport{};
    viewport.width = static_cast<float>(m_pSwapChain->getExtent().width);
    viewport.height = static_cast<float>(m_pSwapChain->getExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{ {0, 0}, m_pSwapChain->getExtent() };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    // Bind descriptor set with G-buffer textures
    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_pPipeline->getLightingPipelineLayout(),
        0, 1, &globalDescriptorSet->getDescriptorSets(),
        0, nullptr);

    // Draw fullscreen quad (3 vertices, no buffers needed)
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    vkCmdEndRendering(commandBuffer);
}