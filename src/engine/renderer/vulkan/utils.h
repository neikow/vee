#ifndef GAME_ENGINE_UTILS_H
#define GAME_ENGINE_UTILS_H
#include <iostream>
#include <vector>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "../../entities/components_system/component_array.h"

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

    VkExtent2D ChooseSwapExtent(const VkExtent2D &targetExtent, const VkSurfaceCapabilitiesKHR &capabilities);

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

    void ReadImagePixel(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue queue,
                        VkCommandPool commandPool, VkImage image, uint32_t width,
                        uint32_t height, uint32_t posX, uint32_t posY, Entities::EntityID entityId);
}

#endif //GAME_ENGINE_UTILS_H
