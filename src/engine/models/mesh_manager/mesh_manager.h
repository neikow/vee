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
};


class MeshManager {
    AbstractRenderer *m_Renderer;
    std::map<ModelId, std::uint32_t> m_ModelIdToMeshIndex;
    std::vector<std::vector<Vertex> > m_Vertices;
    std::vector<std::vector<uint32_t> > m_Indices;
    std::vector<MeshInfo> m_MeshInfos;

    uint32_t m_CurrentVertexOffset = 0;
    uint32_t m_CurrentIndexOffset = 0;

    LoadModelResult LoadModelData(
        const tinyobj::attrib_t &attrib,
        const std::vector<tinyobj::shape_t> &shapes
    );

public:
    explicit MeshManager(AbstractRenderer *renderer) : m_Renderer(renderer) {
    }

    [[nodiscard]] std::vector<Vertex> GetMeshVerticesArray() const {
        return Utils::Vectors::FlattenCopy(m_Vertices);
    }

    [[nodiscard]] std::vector<uint32_t> GetMeshIndicesArray() const {
        return Utils::Vectors::FlattenCopy(m_Indices);
    }

    [[nodiscard]] MeshInfo GetMeshInfo(const ModelId modelId) const {
        return m_MeshInfos[m_ModelIdToMeshIndex.at(modelId)];
    }

    ModelId LoadMesh(const std::string &meshPath);

    void DumpLoadedMeshes(YAML::Emitter &out) const;

    void Reset();
};


#endif //GAME_ENGINE_VULKAN_MESH_MANAGER_H
