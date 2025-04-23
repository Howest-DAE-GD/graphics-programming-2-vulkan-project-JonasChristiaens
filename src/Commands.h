#pragma once

class Device;
class CommandPool;
class Commands
{
public:
	static VkCommandBuffer beginSingleTimeCommands(CommandPool* commandPool, Device* device);
	static void endSingleTimeCommands(VkCommandBuffer commandBuffer, CommandPool* commandPool, Device* device);
};