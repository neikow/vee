#ifndef GAME_ENGINE_UTILS_H
#define GAME_ENGINE_UTILS_H
#include <iostream>
#include <vector>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace Vulkan::Utils {
    bool CheckValidationLayerSupport();

    std::vector<const char *> GetRequiredExtensions();


    VkResult CreateDebugUtilsMessengerEXT(
        const VkInstance &instance,
        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkDebugUtilsMessengerEXT *pDebugMessenger
    );

    void DestroyDebugUtilsMessengerEXT(
        const VkInstance &instance,
        const VkDebugUtilsMessengerEXT &debugMessenger,
        const VkAllocationCallbacks *pAllocator
    );

    bool HasStencilComponent(VkFormat format);

    VkDebugUtilsMessengerCreateInfoEXT GetDebugMessageCreateInfo(
        VkDebugUtilsMessageSeverityFlagsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType
    );

    bool CheckPhysicalDeviceExtensionSupport(const VkPhysicalDevice &physicalDevice);

    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

    VkExtent2D ChooseSwapExtent(GLFWwindow *window, const VkSurfaceCapabilitiesKHR &capabilities);

    VkFormat FindSupportedFormat(
        const VkPhysicalDevice &physicalDevice,
        const std::vector<VkFormat> &candidates,
        VkImageTiling tiling,
        VkFormatFeatureFlags features
    );

    VkFormat FindDepthFormat(
        const VkPhysicalDevice &physicalDevice
    );

    uint32_t FindMemoryType(
        VkPhysicalDevice physicalDevice,
        uint32_t typeFilter,
        VkMemoryPropertyFlags properties
    );
}

#endif //GAME_ENGINE_UTILS_H
