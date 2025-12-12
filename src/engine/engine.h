#ifndef GAME_ENGINE_ENGINE_H
#define GAME_ENGINE_ENGINE_H

#define ENGINE_NAME "Vee Engine"
#define ENGINE_VERSION VK_MAKE_VERSION(1, 0, 0)

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#include "renderer/abstract.h"
#include "scenes/scene.h"

class Engine {
    bool m_ShouldQuit = false;
    bool m_Paused = false;

    std::shared_ptr<Scene> m_Scene;

    std::shared_ptr<AbstractRenderer> m_Renderer;
    EntityID m_ActiveCameraEntityId = NULL_ENTITY;
    std::vector<SystemRegistrationFunction> m_SystemRegistrations;

public:
    explicit Engine(
        const std::shared_ptr<AbstractRenderer> &renderer
    )
        : m_Renderer(renderer) {
    }

    void Initialize(int width, int height, const std::string &appName, uint32_t version) const;

    void UpdateSystems(float deltaTime) const;

    void RegisterSystems(const SystemRegistrationFunction &regFunction);

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

    [[nodiscard]] EntityID GetActiveCameraEntityId() const {
        return m_ActiveCameraEntityId;
    }

    void Pause();

    void Resume();

    void Reset();

    void NewEmptyScene();
};

#endif //GAME_ENGINE_ENGINE_H
