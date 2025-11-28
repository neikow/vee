#ifndef VEE_VERTEX_H
#define VEE_VERTEX_H
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    bool operator==(const Vertex &other) const;
};

template<>
struct std::hash<Vertex> {
    size_t operator()(Vertex const &vertex) const noexcept {
        return ((hash<glm::vec3>()(vertex.pos) ^
                 (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
               (hash<glm::vec2>()(vertex.texCoord) << 1);
    }
};
#endif //VEE_VERTEX_H
