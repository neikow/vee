#ifndef GAME_ENGINE_MESH_COMPONENT_H
#define GAME_ENGINE_MESH_COMPONENT_H
#include <string>

struct RenderableComponent final {
    std::uint32_t meshId;
    std::uint32_t textureId;
};


#endif //GAME_ENGINE_MESH_COMPONENT_H
