#ifndef GAME_ENGINE_CONSTS_H
#define GAME_ENGINE_CONSTS_H

#include <vector>
#include <vulkan/vulkan.h>

#ifndef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
#define VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME "VK_KHR_portability_subset"
#endif

#ifdef NDEBUG
#define ENABLE_VALIDATION_LAYERS false
#else
#define ENABLE_VALIDATION_LAYERS true
#endif

constexpr std::array ACTIVE_VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation"
};

constexpr std::array DEVICE_EXTENSIONS{
    VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#endif //GAME_ENGINE_CONSTS_H
