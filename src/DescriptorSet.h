#pragma once

class Device;
class Texture;
class DescriptorSetLayout;
class DescriptorPool;
class Buffer;
class DescriptorSet
{
public:
	DescriptorSet(Device* device, Texture* texture, DescriptorSetLayout* descriptorSetLayout, DescriptorPool* descriptorPool);
	~DescriptorSet() = default;

	const VkDescriptorSet& getDescriptorSets() const { return m_DescriptorSet; }
	void createDescriptorSets();
	void cleanupDescriptorSet();

	void updateGlobalDescriptorSets(uint32_t textureCount);
	void updateUboDescriptorSets(std::vector<Buffer*>& uniformBuffers);
	void updateGBufferDescriptorSets(VkImageView positionView, VkImageView normalView, VkImageView albedoView, VkImageView materialView);

private:
	Device* m_pDevice;
	Texture* m_pTexture;
	DescriptorSetLayout* m_pDescriptorSetLayout;
	DescriptorPool* m_pDescriptorPool;

	VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
};