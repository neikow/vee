#ifndef GAME_ENGINE_MESH_COMPONENT_H
#define GAME_ENGINE_MESH_COMPONENT_H
#include <string>

struct RenderableComponent final {
    std::uint32_t meshId;
    std::uint32_t textureId;

    RenderableComponent(
        const std::uint32_t meshId,
        const std::uint32_t textureId
    ) : meshId(meshId),
        textureId(textureId) {
    }
};


#endif //GAME_ENGINE_MESH_COMPONENT_H
