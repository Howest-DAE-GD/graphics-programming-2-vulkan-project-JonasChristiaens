#pragma once

class Device;
class SwapChain;
class Renderpass
{
public:
	// constructor & destructor
	Renderpass(Device* device, SwapChain* swapchain);
	~Renderpass() = default;

	// public member functions
	VkRenderPass getRenderPass() const { return m_RenderPass; }

	void createRenderPass();
	void cleanupRenderPass();
private:
	// private member variables
	Device* m_pDevice = nullptr;
	SwapChain* m_pSwapChain = nullptr;
	VkRenderPass m_RenderPass = VK_NULL_HANDLE;
};