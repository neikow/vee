#include "scene.h"

#include "../entities/components_system/components/local_transform_component.h"
#include "../entities/components_system/components/velocity_component.h"
#include "../entities/components_system/components/renderable_component.h"
#include "../entities/components_system/components/camera_component.h"
#include "../entities/components_system/tags/active_camera_tag_component.h"
#include "../entities/components_system/tags/internal_tag_component.h"
#include "../entities/system/display_system.h"
#include "../entities/system/camera_system.h"
#include "../../editor/systems/editor_camera_system.h"
#include "../entities/components_system/components/parent_component.h"
#include "../entities/components_system/components/children_component.h"
#include "../entities/components_system/components/local_to_world_component.h"
#include "../entities/system/movement_system.h"
#include "../entities/system/transform_system.h"

std::string &Scene::GetPath() {
    return m_Path;
}

void Scene::SetPath(const std::string &path) {
    m_Path = path;
}

bool Scene::DestroyEntity(const EntityID entity) const {
    if (m_ComponentManager->HasComponent<InternalTagComponent>(entity)) {
        return false;
    }
    m_EntityManager->RemoveEntity(entity);
    m_ComponentManager->RemoveEntity(entity);
    m_SystemManager->RemoveEntity(entity);
    return true;
}

void Scene::RegisterInternalSystems() {
    Signature movementSignature;
    movementSignature.set(ComponentTypeHelper<LocalTransformComponent>::ID);
    movementSignature.set(ComponentTypeHelper<VelocityComponent>::ID);
    m_SystemManager->RegisterSystem<MovementSystem>(
        std::make_shared<MovementSystem>(m_ComponentManager)
    );
    m_SystemManager->SetSignature<MovementSystem>(movementSignature);

    Signature transformSignature;
    transformSignature.set(ComponentTypeHelper<LocalTransformComponent>::ID);
    transformSignature.set(ComponentTypeHelper<LocalToWorldComponent>::ID);
    m_SystemManager->RegisterSystem<TransformSystem>(
        std::make_shared<TransformSystem>(m_ComponentManager)
    );
    m_SystemManager->SetSignature<TransformSystem>(transformSignature);

    Signature renderableSignature;
    renderableSignature.set(ComponentTypeHelper<RenderableComponent>::ID);
    renderableSignature.set(ComponentTypeHelper<LocalToWorldComponent>::ID);
    m_DisplaySystem = m_SystemManager->RegisterSystem<DisplaySystem>(
        std::make_shared<DisplaySystem>(
            m_Renderer,
            m_ComponentManager,
            m_EntityManager
        )
    );
    m_SystemManager->SetSignature<DisplaySystem>(renderableSignature);
}

void Scene::RegisterInternalComponents() const {
    m_ComponentManager->RegisterComponent<InternalTagComponent>("InternalTagComponent");
    m_ComponentManager->RegisterComponent<ParentComponent>("ParentComponent");
    m_ComponentManager->RegisterComponent<ChildrenComponent>("ChildrenComponent");
    m_ComponentManager->RegisterComponent<LocalTransformComponent>("TransformComponent");
    m_ComponentManager->RegisterComponent<LocalToWorldComponent>("LocalToWorldComponent");
    m_ComponentManager->RegisterComponent<VelocityComponent>("VelocityComponent");
    m_ComponentManager->RegisterComponent<RenderableComponent>("RenderableComponent");
    m_ComponentManager->RegisterComponent<CameraComponent>("CameraComponent");
    m_ComponentManager->RegisterComponent<ActiveCameraTagComponent>("ActiveCameraTagComponent");
}
