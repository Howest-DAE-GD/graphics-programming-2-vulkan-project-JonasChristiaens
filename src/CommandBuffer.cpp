#include "pch.h"
#include "utils.h"
#include "CommandBuffer.h"

#include "Device.h"
#include "CommandPool.h"
#include "Renderpass.h"
#include "SwapChain.h"
#include "Pipeline.h"
#include "Buffer.h"
#include "SceneManager.h"
#include "DescriptorSet.h"
#include <stdexcept>
#include <iostream>

CommandBuffer::CommandBuffer(Device* device, CommandPool* commandPool, Renderpass* renderpass, SwapChain* swapChain, Pipeline* pipeline, std::vector<Buffer*> vertexBuffer,
    std::vector<Buffer*> indexBuffer, SceneManager* sceneManager)
	: m_pDevice(device)
	, m_pCommandPool(commandPool)
    , m_pRenderpass(renderpass)
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

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

