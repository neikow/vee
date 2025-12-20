#include "utils.h"

#include <cstdint>
#include <set>
#include <vector>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "consts.h"
#include "types.h"
#include "vulkan_device.h"

VkBool32 DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                       VkDebugUtilsMessageTypeFlagsEXT messageType,
                       const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                       void *pUserData) {
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        void;
    }
    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}


namespace Vulkan::Utils {
    bool CheckValidationLayerSupport() {
        std::uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char *layerName: ACTIVE_VALIDATION_LAYERS) {
            bool layerFound = false;

            for (const auto &layerProperties: availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    std::vector<const char *> GetRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        auto **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if constexpr (ENABLE_VALIDATION_LAYERS) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

        return extensions;
    }

    VkResult CreateDebugUtilsMessengerEXT(
        const VkInstance &instance,
        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkDebugUtilsMessengerEXT *pDebugMessenger
    ) {
        const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT"));
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(
        const VkInstance &instance,
        const VkDebugUtilsMessengerEXT &debugMessenger,
        const VkAllocationCallbacks *pAllocator
    ) {
        const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    bool HasStencilComponent(const VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT
               || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    bool IsDepthFormat(const VkFormat format) {
        return format == VK_FORMAT_D16_UNORM
               || format == VK_FORMAT_D32_SFLOAT
               || format == VK_FORMAT_D24_UNORM_S8_UINT
               || format == VK_FORMAT_D32_SFLOAT_S8_UINT;
    }

    VkDebugUtilsMessengerCreateInfoEXT GetDebugMessageCreateInfo(
        const VkDebugUtilsMessageSeverityFlagsEXT messageSeverity,
        const VkDebugUtilsMessageTypeFlagsEXT messageType
    ) {
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = messageSeverity;
        createInfo.messageType = messageType;
        createInfo.pfnUserCallback = DebugCallback;

        return createInfo;
    }

    bool CheckPhysicalDeviceExtensionSupport(const VkPhysicalDevice &physicalDevice) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());

        for (const auto &extension: availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
        for (const auto &availableFormat: availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
                && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
        for (const auto &availablePresentMode: availablePresentModes) {
            // Mailbox mode is the best option for low latency and no tearing
            // but it may consume more power
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D ChooseSwapExtent(
        const VkExtent2D &targetExtent,
        const VkSurfaceCapabilitiesKHR &capabilities
    ) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }

        const VkExtent2D extent{
            .width = std::clamp(targetExtent.width, capabilities.minImageExtent.width,
                                capabilities.maxImageExtent.width),
            .height = std::clamp(targetExtent.height, capabilities.minImageExtent.height,
                                 capabilities.maxImageExtent.height)
        };

        return extent;
    }

    VkFormat FindDepthFormat(
        const VkPhysicalDevice &physicalDevice
    ) {
        return FindSupportedFormat(
            physicalDevice,
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    uint32_t FindMemoryType(
        const VkPhysicalDevice &physicalDevice,
        const uint32_t typeFilter,
        const VkMemoryPropertyFlags properties
    ) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) ==
                properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    void CreateImage(
        const std::shared_ptr<VulkanDevice> &device,
        const uint32_t width,
        const uint32_t height,
        const VkFormat format,
        const VkImageTiling tiling,
        const VkImageUsageFlags usage,
        const VmaMemoryUsage vmaUsage,
        VkImage &image,
        VmaAllocation &allocation,
        const char *debugName
    ) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        const VmaAllocationCreateInfo allocInfo = {.usage = vmaUsage};

        vmaCreateImage(device->GetAllocator(), &imageInfo, &allocInfo, &image, &allocation, nullptr);
        vmaSetAllocationName(device->GetAllocator(), allocation, debugName);
    }

    void CopyBufferToImage(
        const VkCommandBuffer &cmd,
        const VkBuffer &buffer,
        const VkImage &image,
        const uint32_t width,
        const uint32_t height
    ) {
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(
            cmd,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );
    }

    VkFormat FindSupportedFormat(
        const VkPhysicalDevice &physicalDevice,
        const std::vector<VkFormat> &candidates,
        const VkImageTiling tiling,
        const VkFormatFeatureFlags features
    ) {
        for (const auto format: candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }

    // void TransitionImageLayout(
    //     const std::shared_ptr<VulkanDevice> &device,
    //     const VkImage &image,
    //     const VkFormat format,
    //     const VkImageLayout oldLayout,
    //     const VkImageLayout newLayout
    // ) {
    //     const auto cmd = device->BeginSingleTimeCommands(
    //         device->GetTransferCommandPool()
    //     );
    //
    //     SyncManager::TransitionResource(
    //         cmd,
    //         image,
    //         format,
    //         oldLayout,
    //         newLayout
    //     );
    //
    //     device->EndSingleTimeCommands(
    //         cmd, device->GetTransferQueue(),
    //         device->GetTransferCommandPool()
    //     );
    // }
}
