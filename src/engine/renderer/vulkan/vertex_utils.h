#ifndef GAME_ENGINE_VULKAN_VERTEX_H
#define GAME_ENGINE_VULKAN_VERTEX_H
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan.h>

namespace Vulkan {
    struct VertexUtils {
        static VkVertexInputBindingDescription GetBindingDescription();

        static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions();

        static std::array<VkVertexInputAttributeDescription, 1> GetPickingAttributeDescriptions();
    };
}


#endif //GAME_ENGINE_VULKAN_VERTEX_H
