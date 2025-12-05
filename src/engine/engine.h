#ifndef GAME_ENGINE_ENGINE_H
#define GAME_ENGINE_ENGINE_H

#define ENGINE_NAME "Engine"
#define ENGINE_VERSION VK_MAKE_VERSION(1, 0, 0)
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#include "renderer/abstract.h"
#include "scenes/scene.h"

class Engine {
    bool m_ShouldQuit = false;
    bool m_Paused = false;

    bool m_IsEditorMode = false;

    std::shared_ptr<Scene> m_Scene;

    std::shared_ptr<AbstractRenderer> m_Renderer;
    EntityID m_ActiveCameraEntityId = NULL_ENTITY;

public:
    explicit Engine(
        const std::shared_ptr<AbstractRenderer> &renderer
    )
        : m_Renderer(renderer) {
    }

    void Initialize(int width, int height, const std::string &appName, uint32_t version) const;

    void UpdateSystems(float deltaTime) const;

    void PrepareForRendering() const;

    void Shutdown() const;

    void LoadScene(const std::string &scenePath);

    [[nodiscard]] bool Paused() const;

    [[nodiscard]] bool ShouldQuit() const;

    [[nodiscard]] std::shared_ptr<AbstractRenderer> GetRenderer() const;

    [[nodiscard]] std::shared_ptr<Scene> GetScene();

    void SetActiveCameraEntityId(const EntityID entityId) {
        m_ActiveCameraEntityId = entityId;
    }

    void Pause();

    void Resume();

    void Reset();

    void ToggleEditorMode(const bool editorMode) {
        m_IsEditorMode = editorMode;
    }
};

#endif //GAME_ENGINE_ENGINE_H
