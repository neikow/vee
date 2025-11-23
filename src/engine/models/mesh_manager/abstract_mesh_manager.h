#ifndef GAME_ENGINE_ABSTRACT_MESH_MANAGER_H
#define GAME_ENGINE_ABSTRACT_MESH_MANAGER_H
#include <string>
#include <unordered_map>
#include <vector>

#include "tiny_obj_loader.h"
#include "../../utils/vectors.h"
#include "../../utils/concepts/hashable.h"

using ModelId = uint32_t;

struct MeshInfo {
    uint32_t indexCount;
    uint32_t vertexOffset;
    uint32_t indexOffset;
};

struct LoadModelResult {
    ModelId modelId;
    uint32_t indexCount;
    uint32_t vertexCount;
};


template<Hashable VertexData>
class IMeshManager {
protected:
    std::vector<std::vector<VertexData> > m_Vertices;
    std::vector<std::vector<uint32_t> > m_Indices;
    std::vector<MeshInfo> m_MeshInfos;

    uint32_t m_CurrentVertexOffset = 0;
    uint32_t m_CurrentIndexOffset = 0;

public:
    virtual ~IMeshManager() = default;

    ModelId LoadModel(const std::string &modelPath) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(
                &attrib,
                &shapes,
                &materials,
                &warn,
                &err,
                modelPath.c_str()
            )
        ) {
            throw std::runtime_error(err);
        }

        auto result = LoadModelData(attrib, shapes);

        MeshInfo meshInfo{};
        meshInfo.indexCount = result.indexCount;
        meshInfo.vertexOffset = m_CurrentVertexOffset;
        meshInfo.indexOffset = m_CurrentIndexOffset;
        m_MeshInfos.push_back(meshInfo);

        m_CurrentVertexOffset += result.vertexCount;
        m_CurrentIndexOffset += result.indexCount;

        return result.modelId;
    };

    std::vector<VertexData> GetMeshVerticesArray() const {
        return Utils::Vectors::FlattenCopy(m_Vertices);
    };

    std::vector<uint32_t> GetMeshIndicesArray() const {
        return Utils::Vectors::FlattenCopy(m_Indices);
    };

    MeshInfo GetMeshInfo(const ModelId modelId) const {
        return m_MeshInfos[modelId];
    };

private:
    virtual LoadModelResult LoadModelData(
        const tinyobj::attrib_t &attrib,
        const std::vector<tinyobj::shape_t> &shapes
    ) = 0;
};

#endif //GAME_ENGINE_ABSTRACT_MESH_MANAGER_H
