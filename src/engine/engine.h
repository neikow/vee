#ifndef GAME_ENGINE_ENGINE_H
#define GAME_ENGINE_ENGINE_H

#define ENGINE_NAME "Engine"
#define ENGINE_VERSION VK_MAKE_VERSION(1, 0, 0)
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#include "entities/system/system_manager.h"
#include "entities/components/component_manager.h"
#include "entities/components/renderable_component.h"
#include "entities/components/tags/active_camera_tag_component.h"
#include "entities/system/camera_system.h"
#include "entities/system/display_system.h"
#include "entities/system/movement_system.h"
#include "models/texture_manager/vulkan_texture_manager.h"
#include "renderer/abstract.h"

class Engine {
    bool m_ShouldQuit = false;

    std::shared_ptr<AbstractRenderer> m_Renderer;

    std::shared_ptr<Vulkan::TextureManager> m_TextureManager;

    std::shared_ptr<SystemManager> m_SystemManager;
    std::shared_ptr<EntityManager> m_EntityManager;

    std::shared_ptr<ComponentManager> m_ComponentManager;

    std::shared_ptr<DisplaySystem> m_DisplaySystem;

public:
    Engine(
        const std::shared_ptr<AbstractRenderer> &renderer,
        const std::shared_ptr<SystemManager> &systemManager,
        const std::shared_ptr<EntityManager> &entityManager,
        const std::shared_ptr<ComponentManager> &componentManager
    )
        : m_Renderer(renderer),
          m_SystemManager(systemManager),
          m_EntityManager(entityManager),
          m_ComponentManager(componentManager) {
        RegisterInternalComponents();
        RegisterInternalSystems();
    }

    void Initialize(int width, int height, const std::string &appName, uint32_t version) const;

    void Update(float deltaTime) const;

    void Shutdown() const;

    [[nodiscard]] bool ShouldQuit() const;

    [[nodiscard]] std::shared_ptr<AbstractRenderer> GetRenderer() const;

    [[nodiscard]] EntityID CreateEntity() const;

private:
    void RegisterInternalSystems();

    void RegisterInternalComponents();
};

#endif //GAME_ENGINE_ENGINE_H
