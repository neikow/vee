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

public:
    std::shared_ptr<AbstractRenderer> m_Renderer;

    std::shared_ptr<Vulkan::TextureManager> m_TextureManager;

    std::shared_ptr<SystemManager> m_SystemManager;
    std::shared_ptr<EntityManager> m_EntityManager;

    std::shared_ptr<ComponentManager> m_ComponentManager;

    std::shared_ptr<DisplaySystem> m_DisplaySystem;
    std::shared_ptr<CameraSystem> m_CameraSystem;
    std::shared_ptr<MovementSystem> m_MovementSystem;

    explicit Engine(
        const std::shared_ptr<AbstractRenderer> &renderer
    )
        : m_Renderer(renderer),
          m_SystemManager(std::make_shared<SystemManager>()),
          m_EntityManager(std::make_shared<EntityManager>()),
          m_ComponentManager(
              std::make_shared<ComponentManager>(
                  m_SystemManager, m_EntityManager)
          ) {
        RegisterComponents();
        RegisterInternalSystems();
    }

    void RegisterComponents() const {
        m_ComponentManager->RegisterComponent<TransformComponent>();
        m_ComponentManager->RegisterComponent<VelocityComponent>();
        m_ComponentManager->RegisterComponent<RenderableComponent>();
        m_ComponentManager->RegisterComponent<CameraComponent>();
        m_ComponentManager->RegisterComponent<ActiveCameraTagComponent>();
    }

    void RegisterInternalSystems() {
        Signature renderableSignature;
        renderableSignature.set(ComponentTypeHelper<RenderableComponent>::ID);
        renderableSignature.set(ComponentTypeHelper<TransformComponent>::ID);
        m_DisplaySystem = m_SystemManager->RegisterSystem<DisplaySystem>(
            std::make_shared<DisplaySystem>(
                m_Renderer,
                m_ComponentManager,
                m_EntityManager
            )
        );
        m_SystemManager->SetSignature<DisplaySystem>(renderableSignature);

        Signature activeCameraSignature;
        activeCameraSignature.set(ComponentTypeHelper<CameraComponent>::ID);
        activeCameraSignature.set(ComponentTypeHelper<ActiveCameraTagComponent>::ID);

        m_CameraSystem = m_SystemManager->RegisterSystem<CameraSystem>(
            std::make_shared<CameraSystem>(
                m_Renderer,
                m_ComponentManager
            )
        );
        m_SystemManager->SetSignature<CameraSystem>(activeCameraSignature);


        Signature movementSignature;
        movementSignature.set(ComponentTypeHelper<TransformComponent>::ID);
        movementSignature.set(ComponentTypeHelper<VelocityComponent>::ID);
        m_MovementSystem = m_SystemManager->RegisterSystem<MovementSystem>(
            std::make_shared<MovementSystem>(m_ComponentManager)
        );
        m_SystemManager->SetSignature<MovementSystem>(movementSignature);
    }

    void Run(const int width, const int height) const {
        m_Renderer->Initialize(
            width,
            height,
            "Vulkan Renderer",
            VK_MAKE_VERSION(1, 0, 0)
        );

        while (!m_ShouldQuit && !m_Renderer->ShouldClose()) {
            glfwPollEvents();

            m_SystemManager->UpdateSystems(0.1f);

            const auto &vikingRoomTransform = m_ComponentManager->GetComponent<TransformComponent>(
                0
            );

            m_DisplaySystem->Render(0.01f);
        }

        m_Renderer->WaitIdle();
        m_Renderer->Cleanup();
    }
};


#endif //GAME_ENGINE_ENGINE_H
