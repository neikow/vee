#ifndef GAME_ENGINE_ABSTRACT_H
#define GAME_ENGINE_ABSTRACT_H
#include <glm/mat4x4.hpp>

#include "imgui.h"
#include "../entities/types.h"
#include "../models/mesh_manager/mesh_manager.h"
#include "../models/texture_manager/vulkan_texture_manager.h"

struct PushData {
    glm::mat4 worldMatrix;
    TextureId textureID;
    Entities::EntityID entityID;
};

using RendererInitTask = std::function<void()>;
using RendererCleanupTask = std::function<void()>;

struct MemoryUsage {
    float availableMemoryMB = 0.0f;
    float usedMemoryMB = 0.0f;
};

class AbstractRenderer {
    std::shared_ptr<MeshManager> m_MeshManager;
    std::shared_ptr<Vulkan::TextureManager> m_TextureManager;

protected:
    std::vector<RendererInitTask> m_InitQueue;
    std::vector<RendererCleanupTask> m_CleanupStack;
    bool m_Initialized = false;

public:
    AbstractRenderer();

    [[nodiscard]] std::shared_ptr<MeshManager> GetMeshManager() const {
        return m_MeshManager;
    }

    [[nodiscard]] std::shared_ptr<Vulkan::TextureManager> GetTextureManager() const {
        return m_TextureManager;
    }

    virtual void Initialize(
        const std::string &appName,
        uint32_t version
    ) = 0;

    [[nodiscard]] bool Initialized() const {
        return m_Initialized;
    }

    virtual MemoryUsage GetMemoryUsage() = 0;

    virtual void Draw() = 0;

    virtual void UpdateCameraMatrix(
        const glm::mat4x4 &viewMatrix,
        const glm::mat4x4 &projectionMatrix
    ) = 0;

    virtual void SubmitDrawCall(
        std::uint32_t entityId,
        const glm::mat4x4 &worldMatrix,
        uint32_t meshId, uint32_t textureId
    ) = 0;

    virtual void Cleanup() = 0;

    virtual void Reset() = 0;

    virtual void WaitIdle() const = 0;

    virtual float GetAspectRatio() = 0;

    virtual void UpdateGeometryBuffers() = 0;

    virtual void UpdateTextureDescriptor(TextureId textureId) = 0;

    void EnqueuePostInitTask(const RendererInitTask &task);

    virtual ~AbstractRenderer() = default;

    virtual void PrepareForRendering() {
    }

protected:
    void ExecuteInitTasks();
};


#endif //GAME_ENGINE_ABSTRACT_H
