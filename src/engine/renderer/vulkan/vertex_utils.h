#ifndef GAME_ENGINE_VULKAN_VERTEX_H
#define GAME_ENGINE_VULKAN_VERTEX_H
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan.h>

namespace Vulkan {
    struct VertexUtils {
        static VkVertexInputBindingDescription GetBindingDescription();

        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

        static std::vector<VkVertexInputAttributeDescription> GetPositionAttributeDescriptions();
    };
}


#endif //GAME_ENGINE_VULKAN_VERTEX_H
