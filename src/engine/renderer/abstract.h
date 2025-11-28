#ifndef GAME_ENGINE_ABSTRACT_H
#define GAME_ENGINE_ABSTRACT_H
#include <glm/mat4x4.hpp>

#include "imgui.h"
#include "../models/mesh_manager/mesh_manager.h"
#include "../models/texture_manager/vulkan_texture_manager.h"

class AbstractRenderer {
    std::shared_ptr<MeshManager> m_MeshManager;
    std::shared_ptr<Vulkan::TextureManager> m_TextureManager;

protected:
    bool m_Initialized = false;

public:
    AbstractRenderer() : m_MeshManager(std::make_shared<MeshManager>()),
                         m_TextureManager(
                             std::make_shared<Vulkan::TextureManager>(this)
                         ) {
    };

    [[nodiscard]] std::shared_ptr<MeshManager> GetMeshManager() const {
        return m_MeshManager;
    }

    [[nodiscard]] std::shared_ptr<Vulkan::TextureManager> GetTextureManager() const {
        ;
        return m_TextureManager;
    }

    virtual void Initialize(int width, int height, const std::string &appName, uint32_t version) = 0;

    [[nodiscard]] bool Initialized() const {
        return m_Initialized;
    }

    virtual void Draw() = 0;

    virtual void UpdateCameraMatrix(
        const glm::mat4x4 &viewMatrix,
        const glm::mat4x4 &projectionMatrix
    ) = 0;

    virtual void SubmitDrawCall(
        const glm::mat4x4 &worldMatrix,
        std::uint32_t meshId,
        std::uint32_t textureId
    ) = 0;

    virtual void Cleanup() = 0;

    virtual void Reset() = 0;

    virtual bool ShouldClose() = 0;

    virtual void WaitIdle() = 0;

    virtual float GetAspectRatio() = 0;

    virtual void SubmitUIDrawData(ImDrawData *drawData) = 0;

    virtual ~AbstractRenderer() = default;
};


#endif //GAME_ENGINE_ABSTRACT_H
