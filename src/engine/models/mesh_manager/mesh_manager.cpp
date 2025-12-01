#include "mesh_manager.h"

#include "../../renderer/abstract.h"

LoadModelResult MeshManager::LoadModelData(
    const tinyobj::attrib_t &attrib,
    const std::vector<tinyobj::shape_t> &shapes
) {
    const auto modelId = static_cast<ModelId>(m_Vertices.size());
    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;
    std::unordered_map<Vertex, uint32_t> uniqueVertices;

    for (const auto &shape: shapes) {
        for (const auto &index: shape.mesh.indices) {
            Vertex vertex{};

            if (index.vertex_index >= 0) {
                size_t vi = static_cast<size_t>(index.vertex_index);
                vertex.pos = {
                    attrib.vertices[3 * vi + 0],
                    attrib.vertices[3 * vi + 1],
                    attrib.vertices[3 * vi + 2]
                };
            } else {
                vertex.pos = {0.0f, 0.0f, 0.0f};
            }

            if (index.texcoord_index >= 0) {
                size_t ti = static_cast<size_t>(index.texcoord_index);
                vertex.texCoord = {
                    attrib.texcoords[2 * ti + 0],
                    1.0f - attrib.texcoords[2 * ti + 1]
                };
            } else {
                vertex.texCoord = {0.0f, 0.0f};
            }

            vertex.color = {1.0f, 1.0f, 1.0f};

            if (!uniqueVertices.contains(vertex)) {
                // Use the current model's vertex count, not the global model count.
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }

    m_Vertices.push_back(std::move(vertices));
    m_Indices.push_back(std::move(indices));

    LoadModelResult result{};
    result.modelId = modelId;
    result.indexCount = static_cast<uint32_t>(m_Indices.back().size());
    result.vertexCount = static_cast<uint32_t>(m_Vertices.back().size());

    return result;
}

ModelId MeshManager::LoadMesh(const std::string &modelPath) {
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

    m_Renderer->EnqueuePostInitTask([this] {
        m_Renderer->UpdateGeometryBuffers();
    });

    m_ModelIdToMeshIndex[result.modelId] = static_cast<uint32_t>(m_MeshInfos.size() - 1);

    return result.modelId;
}

void MeshManager::Reset() {
    m_CurrentVertexOffset = 0;
    m_CurrentIndexOffset = 0;
    m_Vertices.clear();
    m_Indices.clear();
    m_MeshInfos.clear();
    m_ModelIdToMeshIndex.clear();
}
