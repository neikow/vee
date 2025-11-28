#ifndef VEE_SCENE_H
#define VEE_SCENE_H
#include "../entities/manager.h"
#include "../entities/components_system/component_manager.h"
#include "../entities/system/display_system.h"
#include "../renderer/vulkan/vulkan_renderer.h"

namespace Vulkan {
    class TextureManager;
}

class Scene {
    std::shared_ptr<AbstractRenderer> m_Renderer;
    std::shared_ptr<EntityManager> m_EntityManager;
    std::shared_ptr<ComponentManager> m_ComponentManager;
    std::shared_ptr<SystemManager> m_SystemManager;

    std::shared_ptr<DisplaySystem> m_DisplaySystem;

    std::string m_Name = "Untitled Scene";

public:
    explicit Scene(
        const std::shared_ptr<AbstractRenderer> &renderer
    ) : m_Renderer(renderer) {
        m_EntityManager = std::make_shared<EntityManager>();
        m_SystemManager = std::make_shared<SystemManager>();
        m_ComponentManager = std::make_shared<ComponentManager>(m_SystemManager, m_EntityManager);

        RegisterInternalComponents();
        RegisterInternalSystems();
    }

    void SetName(const std::string &name) {
        m_Name = name;
    }

    [[nodiscard]] const std::string &GetName() const {
        return m_Name;
    }

    [[nodiscard]] std::shared_ptr<EntityManager> GetEntityManager() const {
        return m_EntityManager;
    }

    [[nodiscard]] std::shared_ptr<ComponentManager> GetComponentManager() const {
        return m_ComponentManager;
    }

    [[nodiscard]] std::shared_ptr<SystemManager> GetSystemManager() const {
        return m_SystemManager;
    }

    [[nodiscard]] std::shared_ptr<DisplaySystem> GetDisplaySystem() const {
        return m_DisplaySystem;
    }

    [[nodiscard]] EntityID CreateEntity() const {
        return m_EntityManager->CreateEntity();
    }

    std::shared_ptr<AbstractRenderer> GetRenderer() const {
        return m_Renderer;
    }

private:
    void RegisterInternalSystems();

    void RegisterInternalComponents() const;
};


#endif //VEE_SCENE_H
