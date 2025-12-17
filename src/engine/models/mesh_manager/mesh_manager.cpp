#include "mesh_manager.h"

#include "../../renderer/abstract.h"

LoadModelResult MeshManager::LoadModelData(
    const tinyobj::attrib_t &attrib,
    const std::vector<tinyobj::shape_t> &shapes
) {
    const auto modelId = static_cast<ModelId>(m_MeshInfos.size());
    std::unordered_map<Vertex, uint32_t> uniqueVertices;

    auto startVertex = static_cast<uint32_t>(m_GlobalVertices.size());
    auto startIndex = static_cast<uint32_t>(m_GlobalIndices.size());

    uint32_t verticesLoaded = 0;
    uint32_t indicesLoaded = 0;

    for (const auto &shape: shapes) {
        for (const auto &index: shape.mesh.indices) {
            Vertex vertex{};

            if (index.vertex_index >= 0) {
                auto vi = static_cast<size_t>(index.vertex_index);
                vertex.pos = {
                    attrib.vertices[3 * vi + 0],
                    attrib.vertices[3 * vi + 1],
                    attrib.vertices[3 * vi + 2]
                };
            } else {
                vertex.pos = {0.0f, 0.0f, 0.0f};
            }

            if (index.texcoord_index >= 0) {
                auto ti = static_cast<size_t>(index.texcoord_index);
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
                uniqueVertices[vertex] = verticesLoaded;
                m_GlobalVertices.push_back(vertex);
                verticesLoaded++;
            }

            m_GlobalIndices.push_back(uniqueVertices[vertex]);
            indicesLoaded++;
        }
    }

    LoadModelResult result{};
    result.modelId = modelId;
    result.indexOffset = startIndex;
    result.vertexOffset = startVertex;
    result.indexCount = indicesLoaded;
    result.vertexCount = verticesLoaded;

    return result;
}

MeshManager::MeshManager(AbstractRenderer *renderer) : m_Renderer(renderer) {
}

ModelId MeshManager::LoadMesh(const std::string &meshPath) {
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
            meshPath.c_str()
        )
    ) {
        throw std::runtime_error(err);
    }

    auto result = LoadModelData(attrib, shapes);

    MeshInfo meshInfo{};
    meshInfo.path = meshPath;
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

void MeshManager::DumpLoadedMeshes(YAML::Emitter &out) const {
    out << YAML::Key << "meshes" << YAML::Value << YAML::BeginSeq;
    for (int i = 0; i < m_MeshInfos.size(); ++i) {
        out << YAML::BeginMap;
        out << YAML::Key << "id" << YAML::Value << i;
        out << YAML::Key << "path" << YAML::Value << m_MeshInfos[i].path;
        out << YAML::EndMap;
    }
    out << YAML::EndSeq;
}

void MeshManager::Reset() {
    m_CurrentVertexOffset = 0;
    m_CurrentIndexOffset = 0;
    m_GlobalVertices.clear();
    m_GlobalIndices.clear();
    m_MeshInfos.clear();
    m_ModelIdToMeshIndex.clear();
}
