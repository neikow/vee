#ifndef GAME_ENGINE_UTILS_H
#define GAME_ENGINE_UTILS_H
#include <iostream>
#include <vector>
#include <vk_mem_alloc.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "../../entities/components_system/component_array.h"

namespace Vulkan {
    class VulkanDevice;
}

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

    bool IsDepthFormat(VkFormat format);

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
        const VkPhysicalDevice &physicalDevice,
        uint32_t typeFilter,
        VkMemoryPropertyFlags properties
    );

    void CreateImage(
        const std::shared_ptr<VulkanDevice> &device,
        uint32_t width,
        uint32_t height,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VmaMemoryUsage vmaUsage,
        VkImage &image,
        VmaAllocation &allocation,
        const char *debugName
    );

    void CopyBufferToImage(
        const VkCommandBuffer &cmd,
        const VkBuffer &buffer,
        const VkImage &image,
        uint32_t width,
        uint32_t height
    );

    // void TransitionImageLayout(
    //     const std::shared_ptr<VulkanDevice> &device,
    //     const VkImage &image,
    //     VkFormat format,
    //     VkImageLayout oldLayout,
    //     VkImageLayout newLayout
    // );
}

#endif //GAME_ENGINE_UTILS_H
