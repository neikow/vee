#ifndef GAME_ENGINE_VULKAN_MESH_MANAGER_H
#define GAME_ENGINE_VULKAN_MESH_MANAGER_H
#include "abstract_mesh_manager.h"
#include "../../renderer/vulkan/vertex.h"

namespace Vulkan {
    class MeshManager final : public IMeshManager<Vertex> {
        LoadModelResult LoadModelData(
            const tinyobj::attrib_t &attrib,
            const std::vector<tinyobj::shape_t> &shapes
        ) override;
    };
}


#endif //GAME_ENGINE_VULKAN_MESH_MANAGER_H
