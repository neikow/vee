#include "display_system.h"

#include "../../../editor/editor.h"
#include "../../utils/macros/log_macros.h"
#include "../components_system/components/camera_component.h"
#include "../components_system/components/renderable_component.h"
#include "../components_system/component_manager.h"
#include "../components_system/components/local_to_world_component.h"

void DisplaySystem::PrepareCamera(const EntityID cameraEntityId) const {
    if (cameraEntityId == NULL_ENTITY) {
        LOG_ERROR("No valid camera entity ID provided to DisplaySystem::PrepareCamera.");
        return;
    }

    const auto &cameraComponent = m_ComponentManager->GetComponent<CameraComponent>(cameraEntityId);

    m_Renderer->UpdateCameraMatrix(
        cameraComponent.viewMatrix,
        cameraComponent.projectionMatrix
    );
}

void DisplaySystem::SubmitDrawCalls() const {
    for (const auto &entity: m_Entities) {
        const auto &entityTransform = m_ComponentManager->GetComponent<LocalToWorldComponent>(entity);
        const auto &renderable = m_ComponentManager->GetComponent<RenderableComponent>(entity);

        m_Renderer->SubmitDrawCall(
            entity,
            entityTransform.localToWorldMatrix,
            renderable.meshId,
            renderable.textureId
        );
    }
}

void DisplaySystem::PrepareForRendering(const EntityID cameraEntityId) const {
    PrepareCamera(cameraEntityId);

    // TODO: Implement lights

    SubmitDrawCalls();
}
