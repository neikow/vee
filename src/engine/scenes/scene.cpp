#include "scene.h"

#include "../entities/components_system/components/transform_component.h"
#include "../entities/components_system/components/velocity_component.h"
#include "../entities/components_system/components/renderable_component.h"
#include "../entities/components_system/components/camera_component.h"
#include "../entities/components_system/tags/active_camera_tag_component.h"
#include "../entities/components_system/tags/editor_camera_tag_component.h"
#include "../entities/components_system/tags/internal_tag_component.h"
#include "../entities/system/display_system.h"
#include "../entities/system/camera_system.h"
#include "../../editor/systems/editor_camera_system.h"
#include "../entities/system/movement_system.h"

std::string &Scene::GetPath() {
    return m_Path;
}

void Scene::RegisterInternalSystems() {
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

    Signature movementSignature;
    movementSignature.set(ComponentTypeHelper<TransformComponent>::ID);
    movementSignature.set(ComponentTypeHelper<VelocityComponent>::ID);
    m_SystemManager->RegisterSystem<MovementSystem>(
        std::make_shared<MovementSystem>(m_ComponentManager)
    );
    m_SystemManager->SetSignature<MovementSystem>(movementSignature);
}

void Scene::RegisterInternalComponents() const {
    m_ComponentManager->RegisterComponent<InternalTagComponent>();
    m_ComponentManager->RegisterComponent<TransformComponent>();
    m_ComponentManager->RegisterComponent<VelocityComponent>();
    m_ComponentManager->RegisterComponent<RenderableComponent>();
    m_ComponentManager->RegisterComponent<CameraComponent>();
    m_ComponentManager->RegisterComponent<ActiveCameraTagComponent>();
}
