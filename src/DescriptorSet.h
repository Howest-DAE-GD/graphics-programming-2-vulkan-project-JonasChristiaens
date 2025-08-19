#pragma once

class Device;
class Texture;
class DescriptorSetLayout;
class DescriptorPool;
class Buffer;

class DescriptorSet
{
public:
	DescriptorSet(Device* device, Texture* texture, DescriptorSetLayout* descriptorSetLayout, DescriptorPool* descriptorPool, size_t setCount);
	~DescriptorSet() = default;

	const std::vector<VkDescriptorSet>& getDescriptorSets() const { return m_DescriptorSets; } // Return all descriptor sets (for binding per frame)
	VkDescriptorSet getDescriptorSet(size_t frame) const { return m_DescriptorSets[frame]; } // Return descriptor set for a particular frame

	void createDescriptorSets();
	void cleanupDescriptorSet();

	void updateGlobalDescriptorSets(uint32_t textureCount);
	void updateUboDescriptorSets(const std::vector<Buffer*>& uniformBuffers);
	void updateGBufferDescriptorSets(VkImageView positionView, VkImageView normalView, VkImageView albedoView, VkImageView materialView);
	void updateLightingDescriptorSet(VkImageView positionView, VkImageView normalView, VkImageView albedoView, VkImageView materialView,
		VkImageView depthView, VkSampler sampler, VkBuffer cameraBuffer);
	void updateToneMappingDescriptorSet(VkImageView hdrImageView, VkSampler sampler, VkBuffer cameraSettingsBuffer);


private:
	Device* m_pDevice;
	Texture* m_pTexture;
	DescriptorSetLayout* m_pDescriptorSetLayout;
	DescriptorPool* m_pDescriptorPool;

	std::vector<VkDescriptorSet> m_DescriptorSets;
	size_t m_SetCount;
};