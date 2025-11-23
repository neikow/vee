#ifndef GAME_ENGINE_TYPES_H
#define GAME_ENGINE_TYPES_H
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>

namespace Vulkan {
    struct UniformBufferObject {
        glm::mat4 view;
        glm::mat4 proj;
    };


    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> transferFamily;

        [[nodiscard]] bool isComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
}

#endif //GAME_ENGINE_TYPES_H
