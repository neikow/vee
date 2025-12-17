#ifndef GAME_ENGINE_VULKAN_MESH_MANAGER_H
#define GAME_ENGINE_VULKAN_MESH_MANAGER_H
#include "../../renderer/base_vertex.h"
#include "../../utils/vectors.h"
#include "vector"
#include "tiny_obj_loader.h"
#include "yaml-cpp/emitter.h"

class AbstractRenderer;
using ModelId = uint32_t;

struct MeshInfo {
    std::string path;
    uint32_t indexCount;
    uint32_t vertexOffset;
    uint32_t indexOffset;
};

struct LoadModelResult {
    ModelId modelId;
    uint32_t indexCount;
    uint32_t vertexCount;
    uint32_t indexOffset;
    uint32_t vertexOffset;
};


class MeshManager {
    AbstractRenderer *m_Renderer;
    std::map<ModelId, std::uint32_t> m_ModelIdToMeshIndex;
    std::vector<Vertex> m_GlobalVertices;
    std::vector<uint32_t> m_GlobalIndices;
    std::vector<MeshInfo> m_MeshInfos;

    uint32_t m_CurrentVertexOffset = 0;
    uint32_t m_CurrentIndexOffset = 0;

    LoadModelResult LoadModelData(
        const tinyobj::attrib_t &attrib,
        const std::vector<tinyobj::shape_t> &shapes
    );

public:
    explicit MeshManager(AbstractRenderer *renderer);

    [[nodiscard]] std::vector<Vertex> GetMeshVerticesArray() const {
        return m_GlobalVertices;
    }

    [[nodiscard]] std::vector<uint32_t> GetMeshIndicesArray() const {
        return m_GlobalIndices;
    }

    [[nodiscard]] MeshInfo GetMeshInfo(const ModelId modelId) const {
        try {
            const auto meshIndex = m_ModelIdToMeshIndex.at(modelId);
            return m_MeshInfos[meshIndex];
        } catch (const std::out_of_range &) {
            throw std::runtime_error("MeshManager: Invalid ModelId requested: " + std::to_string(modelId));
        }
    }

    ModelId LoadMesh(const std::string &meshPath);

    void DumpLoadedMeshes(YAML::Emitter &out) const;

    void Reset();
};


#endif //GAME_ENGINE_VULKAN_MESH_MANAGER_H
