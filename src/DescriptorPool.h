#pragma once

class Device;
class Texture;
class DescriptorSetLayout;
class Buffer;
class DescriptorPool
{
public:
	// constructor & destructor
	DescriptorPool(Device* device, Texture* texture);
	~DescriptorPool() = default;

	// public member functions
	VkDescriptorPool getDescriptorPool() const { return m_DescriptorPool; }

	void createDescriptorPool();
	void cleanupDescriptorPool();

private:
	// private member variables
	Device* m_pDevice;
	Texture* m_pTexture;
	uint32_t m_maxFramesInFlight;

	VkDescriptorPool m_DescriptorPool{};
};