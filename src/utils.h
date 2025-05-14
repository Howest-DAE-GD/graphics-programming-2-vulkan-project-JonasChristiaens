#pragma once
#include <string>
#include <array>
#include <vector>
#include <optional>

// Constants
const std::string MODEL_PATH{ "./models/viking_room.obj" };
const std::string TEXTURE_PATH{ "./textures/viking_room.png" };

const int MAX_FRAMES_IN_FLIGHT{ 2 };

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const float aspectRatio{ static_cast<float>(WIDTH) / static_cast<float>(HEIGHT) };

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    //VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// Structs
struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily{};
	std::optional<uint32_t> presentFamily{};

    bool isComplete() const;
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats{};
    std::vector<VkPresentModeKHR> presentModes{};
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();

    bool operator==(const Vertex& other) const;
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const;
    };
}

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

// functions
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
std::vector<char> readFile(const std::string& filename);