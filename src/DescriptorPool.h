#pragma once

class Device;
class Texture;
class DescriptorSetLayout;
class Buffer;
class DescriptorPool
{
public:
	// constructor & destructor
	DescriptorPool(Device* device, Texture* texture, DescriptorSetLayout* descriptorSetLayout, std::vector<Buffer*>& uniformBuffers);
	~DescriptorPool() = default;

	// public member functions
	VkDescriptorPool getDescriptorPool() const { return m_DescriptorPool; }
	const std::vector<VkDescriptorSet>& getDescriptorSets() const { return m_DescriptorSets; }

	void createDescriptorPool();
	void createDescriptorSets(std::vector<Buffer*>& uniformBuffers);

	void cleanupDescriptorPool();

private:
	// private member variables
	Device* m_pDevice;
	Texture* m_pTexture;
	DescriptorSetLayout* m_pDescriptorSetLayout;
	uint32_t m_maxFramesInFlight;

	VkDescriptorPool m_DescriptorPool{};
	std::vector<VkDescriptorSet> m_DescriptorSets{};


	// private member functions

};