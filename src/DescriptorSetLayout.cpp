#include "pch.h"
#include "DescriptorSetLayout.h"

#include "Device.h"
#include <stdexcept>

DescriptorSetLayout::DescriptorSetLayout(Device* device)
	: m_pDevice(device)
{
}

void DescriptorSetLayout::cleanupDescriptorSetLayout()
{
    if (m_DescriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(m_pDevice->getDevice(), m_DescriptorSetLayout, nullptr);
        m_DescriptorSetLayout = VK_NULL_HANDLE;
    }
}

// Provide details about every descriptor binding used in shaders for pipeline creation
void DescriptorSetLayout::createGlobalDescriptorSetLayout(uint32_t textureCount, bool includeGBuffers)
{
    std::vector<VkDescriptorSetLayoutBinding> bindings;

    // Binding 0: Sampler (shared sampler for all textures)
    bindings.push_back({
        0, // binding
        VK_DESCRIPTOR_TYPE_SAMPLER,
        1, // descriptorCount
        VK_SHADER_STAGE_FRAGMENT_BIT,
        nullptr // pImmutableSamplers
        });

    // Binding 1: Texture array (sampled images)
    bindings.push_back({
        1, // binding
        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        textureCount,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        nullptr
        });

    // Add G-buffer bindings if needed
    if (includeGBuffers)
    {
        // Binding 2: G-Buffer Position (RGBA16F)
        bindings.push_back({
            2,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            1,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            nullptr
            });

        // Binding 3: G-Buffer Normal (RG16_SNORM)
        bindings.push_back({
            3,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            1,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            nullptr
            });

        // Binding 4: G-Buffer Albedo (RGBA8_UNORM)
        bindings.push_back({
            4,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            1,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            nullptr
            });

        // Binding 5: G-Buffer Material (RG8_UNORM)
        bindings.push_back({
            5,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            1,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            nullptr
            });
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(m_pDevice->getDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor set layout!");
}

void DescriptorSetLayout::createUboDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    std::array<VkDescriptorSetLayoutBinding, 1> bindings = { uboLayoutBinding };

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(m_pDevice->getDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor set layout!");
}