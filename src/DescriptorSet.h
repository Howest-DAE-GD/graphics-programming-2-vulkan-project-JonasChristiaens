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

	void updateGlobalDescriptorSets(uint32_t textureCount);
	void updateUboDescriptorSets(std::vector<Buffer*>& uniformBuffers);

private:
	Device* m_pDevice;
	Texture* m_pTexture;
	DescriptorSetLayout* m_pDescriptorSetLayout;
	DescriptorPool* m_pDescriptorPool;

	VkDescriptorSet m_DescriptorSet;
};