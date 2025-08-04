#include "pch.h"
#include "utils.h"
#include "SceneManager.h"

#include "Device.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "Window.h"
#include "CommandBuffer.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdexcept>
#include <vector>
#include <chrono>
#include <iostream>

SceneManager::SceneManager(Device* device, SwapChain* spawChain, Renderpass* renderpass)
    : m_pDevice(device)
    , m_pSwapChain(spawChain)
    , m_pRenderpass(renderpass)
{
}

void SceneManager::loadModel()
{
    Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(MODEL_PATH, 
        aiProcess_Triangulate | 
		aiProcess_CalcTangentSpace |
		aiProcess_JoinIdenticalVertices |
        aiProcess_FlipUVs
    );

    // check for failure
    if (!scene)
    {
		std::cout << "Error loading model: " << importer.GetErrorString() << std::endl;
        return;
    }

	m_texturePaths.resize(scene->mNumMaterials);
	m_normalPaths.resize(scene->mNumMaterials);
    
    const aiMatrix4x4 root = scene->mRootNode->mTransformation;
    for (unsigned int m = 0; m < scene->mNumMeshes; m++)
    {
        aiMesh* mesh = scene->mMeshes[m];
		Mesh newMesh;

		// Process vertices
        newMesh.vertices.reserve(mesh->mNumVertices);
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex v;
            aiVector3D scaleVertices = root * mesh->mVertices[i];
            v.pos = glm::vec3(scaleVertices.x, scaleVertices.y, scaleVertices.z);
            v.texCoord = mesh->HasTextureCoords(0) ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : glm::vec2(0.0f);
            v.color = { 1.f, 1.f, 1.f };
            v.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            v.tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
            v.biTangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
            newMesh.vertices.push_back(v);
        }

        // Process indices
		newMesh.indices.reserve(mesh->mNumFaces * 3);
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                newMesh.indices.push_back(face.mIndices[j]);
            }
        }

        // Store material index
        newMesh.materialIndex = mesh->mMaterialIndex;
		m_Meshes.push_back(newMesh);

        // load material textures
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        aiString texPath;

        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
            m_texturePaths[mesh->mMaterialIndex] = "models\\" + std::string(texPath.C_Str());
        }

        if (material->GetTexture(aiTextureType_NORMALS, 0, &texPath) == AI_SUCCESS) {
            m_normalPaths[mesh->mMaterialIndex] = "models\\" + std::string(texPath.C_Str());
        }
    }

}

void SceneManager::drawFrame(Window* window, std::vector<void*> uniformBuffersMapped, CommandBuffer* commandBuffers, DescriptorSet* globalDescriptorSet, std::vector<DescriptorSet*> uboDescriptorSets)
{
    VkCommandBuffer rawCommandBuffer{ commandBuffers->getCommandBuffers()[m_CurrentFrame] };
    vkWaitForFences(m_pDevice->getDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(m_pDevice->getDevice(), m_pSwapChain->getSwapChain(), UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        m_pSwapChain->recreateSwapChain(m_pRenderpass);
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    updateUniformBuffer(m_CurrentFrame, uniformBuffersMapped);

    vkResetFences(m_pDevice->getDevice(), 1, &m_InFlightFences[m_CurrentFrame]);

    vkResetCommandBuffer(commandBuffers->getCommandBuffers()[m_CurrentFrame], 0);
    commandBuffers->recordCommandBuffer(commandBuffers->getCommandBuffers()[m_CurrentFrame], imageIndex, m_Meshes, globalDescriptorSet, uboDescriptorSets);

    VkCommandBufferSubmitInfo commandBufferSubmitInfo{};
    commandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    commandBufferSubmitInfo.commandBuffer = rawCommandBuffer;

    VkSemaphoreSubmitInfo waitSemaphoreSubmitInfo{};
    waitSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    waitSemaphoreSubmitInfo.semaphore = m_ImageAvailableSemaphores[m_CurrentFrame];
    waitSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSemaphoreSubmitInfo signalSemaphoreSubmitInfo{};
    signalSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    signalSemaphoreSubmitInfo.semaphore = m_RenderFinishedSemaphores[m_CurrentFrame];
    signalSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    VkSubmitInfo2 submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &commandBufferSubmitInfo;
    submitInfo.waitSemaphoreInfoCount = 1;
    submitInfo.pWaitSemaphoreInfos = &waitSemaphoreSubmitInfo;
    submitInfo.signalSemaphoreInfoCount = 1;
    submitInfo.pSignalSemaphoreInfos = &signalSemaphoreSubmitInfo;

    if (vkQueueSubmit2(m_pDevice->getGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_RenderFinishedSemaphores[m_CurrentFrame];

    VkSwapchainKHR swapChains[] = { m_pSwapChain->getSwapChain() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(m_pDevice->getPresentQueue(), &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window->wasWindowResized()) {
        window->resetWindowResizedFlag();
        m_pSwapChain->recreateSwapChain(m_pRenderpass);
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void SceneManager::createSyncObjects()
{
    m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(m_pDevice->getDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_pDevice->getDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_pDevice->getDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS) {

            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void SceneManager::cleanupScene()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(m_pDevice->getDevice(), m_RenderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_pDevice->getDevice(), m_ImageAvailableSemaphores[i], nullptr);
        vkDestroyFence(m_pDevice->getDevice(), m_InFlightFences[i], nullptr);
    }
}

// Generate a new transformation every frame to make geometry spin around
void SceneManager::updateUniformBuffer(uint32_t currentImage, std::vector<void*> uniformBuffersMapped)
{
    // Calculate time in seconds
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    // Define model, view and projection transformations in UBO
    UniformBufferObject ubo{};
    ubo.model = glm::scale(glm::mat4(1.0), glm::vec3(.01f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), m_pSwapChain->getExtent().width / (float)m_pSwapChain->getExtent().height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    // Copy data in UBO to current uniform buffer (! without staging buffer)
    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}
