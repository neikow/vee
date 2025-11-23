#ifndef GAME_ENGINE_VERTEX_H
#define GAME_ENGINE_VERTEX_H
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan.h>

namespace Vulkan {
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;

        bool operator==(const Vertex &other) const;

        static VkVertexInputBindingDescription getBindingDescription();

        static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
    };
}

namespace std {
    template<>
    struct hash<Vulkan::Vertex> {
        size_t operator()(Vulkan::Vertex const &vertex) const noexcept {
            return ((hash<glm::vec3>()(vertex.pos) ^
                     (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}


#endif //GAME_ENGINE_VERTEX_H
