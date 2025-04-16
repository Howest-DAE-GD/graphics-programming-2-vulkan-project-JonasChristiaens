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
	Device* m_pDevice{};
	SwapChain* m_pSwapChain{};
	VkRenderPass m_RenderPass{};
};