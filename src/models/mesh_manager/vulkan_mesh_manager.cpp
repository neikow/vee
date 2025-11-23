#include "vulkan_mesh_manager.h"

namespace Vulkan {
    LoadModelResult ModelManager::LoadModelData(
        const tinyobj::attrib_t &attrib,
        const std::vector<tinyobj::shape_t> &shapes
    ) {
        const ModelId modelId = static_cast<ModelId>(m_Vertices.size());
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
}
