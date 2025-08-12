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

void CommandBuffer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, std::vector<Mesh> Meshes, DescriptorSet* globalDescriptorSet, std::vector<DescriptorSet*> uboDescriptorSets)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("failed to begin recording command buffer!");

    VkClearValue depthClearValue = { .depthStencil = { 1.0f, 0 } };

    // Manual layout transitions for color image
    VkImageMemoryBarrier colorBarrier{};
    colorBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    colorBarrier.srcAccessMask = 0;
    colorBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    colorBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    colorBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    colorBarrier.image = m_pSwapChain->getSwapChainImages()[imageIndex];
    colorBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    colorBarrier.subresourceRange.baseMipLevel = 0;
    colorBarrier.subresourceRange.levelCount = 1;
    colorBarrier.subresourceRange.baseArrayLayer = 0;
    colorBarrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0, 0, nullptr, 0, nullptr, 1, &colorBarrier);
    
    VkImageMemoryBarrier depthInitBarrier{};
    depthInitBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    depthInitBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthInitBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthInitBarrier.srcAccessMask = 0;
    depthInitBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    depthInitBarrier.image = m_pSwapChain->m_pImage->getDepthImage();
    depthInitBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthInitBarrier.subresourceRange.baseMipLevel = 0;
    depthInitBarrier.subresourceRange.levelCount = 1;
    depthInitBarrier.subresourceRange.baseArrayLayer = 0;
    depthInitBarrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &depthInitBarrier);

    // ---- Depth Prepass ----
    VkRenderingAttachmentInfoKHR depthPrepassAttachment = {};
    depthPrepassAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    depthPrepassAttachment.imageView = m_pSwapChain->m_pImage->getDepthImageView();
    depthPrepassAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthPrepassAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthPrepassAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthPrepassAttachment.clearValue = depthClearValue;

    VkRenderingInfoKHR depthPrepassInfo = {};
    depthPrepassInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    depthPrepassInfo.renderArea.offset = { 0, 0 };
    depthPrepassInfo.renderArea.extent = m_pSwapChain->getExtent();
    depthPrepassInfo.layerCount = 1;
    depthPrepassInfo.colorAttachmentCount = 0;
    depthPrepassInfo.pColorAttachments = nullptr;
    depthPrepassInfo.pDepthAttachment = &depthPrepassAttachment;

    vkCmdBeginRendering(commandBuffer, &depthPrepassInfo);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pPipeline->getDepthPrepassPipeline());

        // Set dynamic viewport/scissor (required for dynamic state pipelines)
        VkViewport depthVp{};
        depthVp.x = 0.0f;
        depthVp.y = 0.0f;
        depthVp.width = static_cast<float>(m_pSwapChain->getExtent().width);
        depthVp.height = static_cast<float>(m_pSwapChain->getExtent().height);
        depthVp.minDepth = 0.0f;
        depthVp.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &depthVp);

        VkRect2D depthScr{};
        depthScr.offset = { 0, 0 };
        depthScr.extent = m_pSwapChain->getExtent();
        vkCmdSetScissor(commandBuffer, 0, 1, &depthScr);

        // Bind only UBO descriptors (not textures/materials)
        for (int i = 0; i < m_pIndexBuffers.size(); ++i) {
            // Bind vertex and index buffers
            VkBuffer vertexBuffers[] = { m_pVertexBuffers[i]->getBuffer() };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffer, m_pIndexBuffers[i]->getBuffer(), 0, VK_INDEX_TYPE_UINT32);

            // Bind BOTH descriptor sets (global and UBO)
            VkDescriptorSet descriptorSets[] = {
                globalDescriptorSet->getDescriptorSets(),
                uboDescriptorSets[m_pSceneManager->getCurrentFrame()]->getDescriptorSets()
            };

            vkCmdBindDescriptorSets(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_pPipeline->getDepthPrepassLayout(),
                0, // firstSet
                2, // descriptorSetCount
                descriptorSets,
                0, nullptr
            );

            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(Meshes[i].indices.size()), 1, 0, 0, 0);
        }
    vkCmdEndRendering(commandBuffer);

    VkImageMemoryBarrier depthBarrier{};
    depthBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    depthBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    depthBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    depthBarrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
    depthBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    depthBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    depthBarrier.image = m_pSwapChain->m_pImage->getDepthImage();
    depthBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthBarrier.subresourceRange.baseMipLevel = 0;
    depthBarrier.subresourceRange.levelCount = 1;
    depthBarrier.subresourceRange.baseArrayLayer = 0;
    depthBarrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &depthBarrier);

    // ---- Main pass ----
    // Setup dynamic rendering structure
    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView = m_pSwapChain->getImageViews()[imageIndex];
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    VkClearValue clearValue = { .color = { {1.0f, 0.0f, 0.0f, 1.0f} } };
    colorAttachment.clearValue = clearValue;

    VkRenderingAttachmentInfoKHR depthAttachment{};
    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    depthAttachment.imageView = m_pSwapChain->m_pImage->getDepthImageView();
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.clearValue = depthClearValue;

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea.offset = { 0, 0 };
    renderingInfo.renderArea.extent = m_pSwapChain->getExtent();
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;
	renderingInfo.pDepthAttachment = &depthAttachment;

    vkCmdBeginRendering(commandBuffer, &renderingInfo);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pPipeline->getGraphicsPipeline());

        VkViewport graphicsVp{};
        graphicsVp.x = 0.0f;
        graphicsVp.y = 0.0f;
        graphicsVp.width = static_cast<float>(m_pSwapChain->getExtent().width);
        graphicsVp.height = static_cast<float>(m_pSwapChain->getExtent().height);
        graphicsVp.minDepth = 0.0f;
        graphicsVp.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &graphicsVp);

        VkRect2D graphicsScr{};
        graphicsScr.offset = { 0, 0 };
        graphicsScr.extent = m_pSwapChain->getExtent();
        vkCmdSetScissor(commandBuffer, 0, 1, &graphicsScr);

        // Bind index buffer
        for (int i{}; i < m_pIndexBuffers.size(); ++i)
        {
            VkBuffer vertexBuffers[]{ m_pVertexBuffers[i]->getBuffer() };
            VkDeviceSize offsets[]{ 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffer, m_pIndexBuffers[i]->getBuffer(), 0, VK_INDEX_TYPE_UINT32);

            // Bind descriptor set
            // global
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pPipeline->getPipelineLayout(), 0, 1, &globalDescriptorSet->getDescriptorSets(), 0, nullptr);

            // ubo
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pPipeline->getPipelineLayout(), 1, 1, &uboDescriptorSets[m_pSceneManager->getCurrentFrame()]->getDescriptorSets(), 0, nullptr);

            // push constants
            vkCmdPushConstants(commandBuffer, m_pPipeline->getPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t), &Meshes[i].materialIndex);

            // New draw to use indexbuffer
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(Meshes[i].indices.size()), 1, 0, 0, 0);
        }

    vkCmdEndRendering(commandBuffer);

    VkImageMemoryBarrier presentBarrier{};
    presentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    presentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    presentBarrier.dstAccessMask = 0;
    presentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    presentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    presentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    presentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    presentBarrier.image = m_pSwapChain->getSwapChainImages()[imageIndex];
    presentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    presentBarrier.subresourceRange.baseMipLevel = 0;
    presentBarrier.subresourceRange.levelCount = 1;
    presentBarrier.subresourceRange.baseArrayLayer = 0;
    presentBarrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0, 0, nullptr, 0, nullptr, 1, &presentBarrier);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}