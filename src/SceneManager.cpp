#include "pch.h"
#include "utils.h"
#include "SceneManager.h"

#include "Device.h"
#include "SwapChain.h"
#include "Window.h"
#include "CommandBuffer.h"
#include "DescriptorSet.h"
#include "DescriptorSetLayout.h"
#include "Buffer.h"
#include "Pipeline.h"
#include "camera.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdexcept>
#include <vector>
#include <chrono>
#include <iostream>
#include <filesystem>

SceneManager::SceneManager(Device* device, SwapChain* spawChain)
    : m_pDevice(device)
    , m_pSwapChain(spawChain)
{
}

void SceneManager::loadModel()
{
    Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(
        MODEL_PATH, 
        aiProcess_Triangulate | 
		aiProcess_CalcTangentSpace |
		aiProcess_JoinIdenticalVertices |
        aiProcess_FlipUVs |
        aiProcess_GenSmoothNormals
    );

    // check for failure
    if (!scene || !scene->HasMeshes())
    {
		std::cout << "Error loading model: " << importer.GetErrorString() << std::endl;
        return;
    }

    // Prepare per-material texture path vectors
    size_t numMaterials = scene->mNumMaterials -1;
    m_albedoPaths.assign(numMaterials, "");
    m_normalPaths.assign(numMaterials, "");
    m_metallicRoughnessPaths.assign(numMaterials, "");

    std::filesystem::path basePath = std::filesystem::path(MODEL_PATH).parent_path();

    m_Meshes.clear();

    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        aiMesh* mesh = scene->mMeshes[m];
        Mesh newMesh;
        newMesh.materialIndex = mesh->mMaterialIndex;

        // Vertices
        newMesh.vertices.reserve(mesh->mNumVertices);
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            Vertex v;
            v.pos = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            v.color = glm::vec3(1.0f);
            v.texCoord = mesh->HasTextureCoords(0)
                ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y)
                : glm::vec2(0.0f);
            v.normal = mesh->HasNormals()
                ? glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z)
                : glm::vec3(0.0f);
            v.tangent = mesh->HasTangentsAndBitangents()
                ? glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z)
                : glm::vec3(0.0f);
            v.biTangent = mesh->HasTangentsAndBitangents()
                ? glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z)
                : glm::vec3(0.0f);
            newMesh.vertices.push_back(v);
        }

        // Indices
        newMesh.indices.reserve(mesh->mNumFaces * 3);
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; ++j) {
                newMesh.indices.push_back(face.mIndices[j]);
            }
        }

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        aiString texPath;

        // Albedo/BaseColor
        if (material->GetTexture(aiTextureType_BASE_COLOR, 0, &texPath) == AI_SUCCESS) {
            m_albedoPaths[newMesh.materialIndex] = (basePath / texPath.C_Str()).string();
        }
        else if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
            m_albedoPaths[newMesh.materialIndex] = (basePath / texPath.C_Str()).string();
        }

        // Normal Map
        if (material->GetTexture(aiTextureType_NORMALS, 0, &texPath) == AI_SUCCESS) {
            m_normalPaths[newMesh.materialIndex] = (basePath / texPath.C_Str()).string();
        }
        else if (material->GetTexture(aiTextureType_HEIGHT, 0, &texPath) == AI_SUCCESS) {
            m_normalPaths[newMesh.materialIndex] = (basePath / texPath.C_Str()).string();
        }

        // Metallic-Roughness Map (glTF standard: aiTextureType_UNKNOWN)
        if (material->GetTexture(aiTextureType_UNKNOWN, 0, &texPath) == AI_SUCCESS) {
            m_metallicRoughnessPaths[newMesh.materialIndex] = (basePath / texPath.C_Str()).string();
        }

        m_Meshes.push_back(newMesh);
    }

    std::cout << "Loaded " << m_Meshes.size() << " meshes from " << MODEL_PATH << std::endl;

    // Debug: print found texture paths per material
    for (size_t i = 0; i < numMaterials; ++i) {
        std::cout << "Material " << i << ":\n";
        std::cout << "  Albedo: " << m_albedoPaths[i] << "\n";
        std::cout << "  Normal: " << m_normalPaths[i] << "\n";
        std::cout << "  MetallicRoughness: " << m_metallicRoughnessPaths[i] << "\n";
    }
}

void SceneManager::drawFrame(Window* window, std::vector<void*> uniformBuffersMapped, CommandBuffer* commandBuffers, DescriptorSet* globalDescriptorSet, DescriptorSet* uboDescriptorSet,
    DescriptorSetLayout* globalDescriptorSetLayout, DescriptorSetLayout* uboDescriptorSetLayout, std::vector<Buffer*>& uniformBuffers, Pipeline* pipeline, Camera* camera)
{
    VkCommandBuffer rawCommandBuffer{ commandBuffers->getCommandBuffers()[m_CurrentFrame] };
    vkWaitForFences(m_pDevice->getDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(m_pDevice->getDevice(), m_pSwapChain->getSwapChain(), UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        m_pSwapChain->recreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    updateUniformBuffer(m_CurrentFrame, uniformBuffersMapped, camera);

    vkResetFences(m_pDevice->getDevice(), 1, &m_InFlightFences[m_CurrentFrame]);

    vkResetCommandBuffer(commandBuffers->getCommandBuffers()[m_CurrentFrame], 0);
    commandBuffers->recordDeferredCommandBuffer(commandBuffers->getCommandBuffers()[m_CurrentFrame], imageIndex, m_Meshes, globalDescriptorSet, uboDescriptorSet);

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
        m_pSwapChain->recreateSwapChain();
        recreateDependentResources(globalDescriptorSetLayout, uboDescriptorSetLayout, globalDescriptorSet, uboDescriptorSet, uniformBuffers, pipeline);
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void SceneManager::recreateDependentResources(DescriptorSetLayout* globalDescriptorSetLayout, DescriptorSetLayout* uboDescriptorSetLayout, DescriptorSet* globalDescriptorSet, 
    DescriptorSet* uboDescriptorSet, std::vector<Buffer*>& uniformBuffers, Pipeline* pipeline)
{
    vkDeviceWaitIdle(m_pDevice->getDevice());

    // 1. Update Global descriptor sets
    globalDescriptorSet->updateGlobalDescriptorSets(static_cast<uint32_t>(m_albedoPaths.size()));

    // 2. Update G-buffer descriptors with new image views
    const GBuffer& gBuffer = m_pSwapChain->getGBuffer();
    globalDescriptorSet->updateGBufferDescriptorSets(
        gBuffer.views[0], // Position
        gBuffer.views[1], // Normal
        gBuffer.views[2], // Albedo
        gBuffer.views[3]  // Material
    );

    // 3. Update UBO descriptor set
	uboDescriptorSet->updateUboDescriptorSets(uniformBuffers);
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

    m_RenderFinishedSemaphores.clear();
    m_ImageAvailableSemaphores.clear();
    m_InFlightFences.clear();
}

// Generate a new transformation every frame to make geometry spin around
void SceneManager::updateUniformBuffer(uint32_t currentImage, std::vector<void*> uniformBuffersMapped, Camera* pCamera)
{
    // Define model, view and projection transformations in UBO
    UniformBufferObject ubo{};
    ubo.model = glm::mat4(1.0f);
    ubo.model = glm::scale(ubo.model, glm::vec3(0.01f));

    ubo.view = pCamera->getViewMatrix();
    ubo.proj = pCamera->getProjectionMatrix();
    ubo.proj[1][1] *= -1;

    // Copy data in UBO to current uniform buffer (! without staging buffer)
    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}