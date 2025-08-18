#pragma once
#include <string>
#include <array>
#include <vector>
#include <optional>

// Constants
const std::string MODEL_PATH{ "./models/glTF/Sponza.gltf" };

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
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 biTangent;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 6> getAttributeDescriptions();
    static std::array<VkVertexInputAttributeDescription, 2> getDepthPrepassAttributeDescriptions();

    bool operator==(const Vertex& other) const;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    uint32_t materialIndex;

    size_t vertexCount() const { return vertices.size(); }

    size_t indexCount() const { return indices.size(); }
	std::vector<uint32_t> getIndices() const { return indices; }
};

struct UniformBufferObject {
	// Model, view and projection matrices
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;

	// Camera position and inverse matrices for lighting calculations
    alignas(16) glm::vec3 cameraPos;
    alignas(16) glm::mat4 invView;
    alignas(16) glm::mat4 invProj;
};

struct GBuffer {
    VkImage position;
    VkDeviceMemory positionMemory;
    VkImage normal;
    VkDeviceMemory normalMemory;
    VkImage albedo;
    VkDeviceMemory albedoMemory;
    VkImage material;
    VkDeviceMemory materialMemory;
    VkImageView views[4];
    VkFramebuffer framebuffer;
    VkRenderPass renderPass;

    void cleanup(VkDevice device) {
        // Destroy all resources
        for (int i = 0; i < 4; i++) {
            if (views[i] != VK_NULL_HANDLE) {
                vkDestroyImageView(device, views[i], nullptr);
            }
        }

        if (position != VK_NULL_HANDLE) vkDestroyImage(device, position, nullptr);
        if (normal != VK_NULL_HANDLE) vkDestroyImage(device, normal, nullptr);
        if (albedo != VK_NULL_HANDLE) vkDestroyImage(device, albedo, nullptr);
        if (material != VK_NULL_HANDLE) vkDestroyImage(device, material, nullptr);

        if (positionMemory != VK_NULL_HANDLE) vkFreeMemory(device, positionMemory, nullptr);
        if (normalMemory != VK_NULL_HANDLE) vkFreeMemory(device, normalMemory, nullptr);
        if (albedoMemory != VK_NULL_HANDLE) vkFreeMemory(device, albedoMemory, nullptr);
        if (materialMemory != VK_NULL_HANDLE) vkFreeMemory(device, materialMemory, nullptr);

        if (framebuffer != VK_NULL_HANDLE) vkDestroyFramebuffer(device, framebuffer, nullptr);
        if (renderPass != VK_NULL_HANDLE) vkDestroyRenderPass(device, renderPass, nullptr);
    }
};

struct CameraSettingsUBO {
    float aperture;      // f-stop
    float shutterSpeed;  // seconds
    float ISO;           // ISO value
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const;
    };
}

// functions
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
std::vector<char> readFile(const std::string& filename);