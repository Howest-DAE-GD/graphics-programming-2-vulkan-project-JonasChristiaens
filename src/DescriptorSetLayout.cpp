#include "pch.h"
#include "DescriptorSetLayout.h"

#include "Device.h"
#include <stdexcept>

DescriptorSetLayout::DescriptorSetLayout(Device* device)
	: m_pDevice(device)
{
	createDescriptorSetLayout();
}

void DescriptorSetLayout::cleanupDescriptorSetLayout()
{
    if (m_DescriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(m_pDevice->getDevice(), m_DescriptorSetLayout, nullptr);
        m_DescriptorSetLayout = VK_NULL_HANDLE;
    }
}

// Provide details about every descriptor binding used in shaders for pipeline creation
void DescriptorSetLayout::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0; // binding used in shader
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // type of of descriptor
    uboLayoutBinding.descriptorCount = 1; // nr of values in array
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // specify in which shader stage descriptor will be referenced

    // combined image sampler descriptor
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
    // descriptor set layout has to be specified during pipeline creation to set which descriptors the shaders will be using
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(m_pDevice->getDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor set layout!");
}
