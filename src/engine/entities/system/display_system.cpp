#include "display_system.h"

#include "../../../editor/editor.h"
#include "../../utils/math_utils.h"
#include "../components_system/components/transform_component.h"
#include "../components_system/components/camera_component.h"
#include "../components_system/components/renderable_component.h"
#include "../components_system/component_manager.h"
#include "../components_system/tags/active_camera_tag_component.h"

void DisplaySystem::PrepareCamera(const EntityID cameraEntityId) const {
    if (cameraEntityId == NULL_ENTITY) {
        throw std::runtime_error("No camera to prepare.");
    }

    const auto &cameraComponent = m_ComponentManager->GetComponent<CameraComponent>(cameraEntityId);

    m_Renderer->UpdateCameraMatrix(
        cameraComponent.viewMatrix,
        cameraComponent.projectionMatrix
    );
}

void DisplaySystem::SubmitDrawCalls() const {
    for (const auto &entity: m_Entities) {
        const auto &transform = m_ComponentManager->GetComponent<TransformComponent>(entity);
        const auto &renderable = m_ComponentManager->GetComponent<RenderableComponent>(entity);

        glm::mat4x4 worldMatrix = Utils::Math::CalculateWorldMatrix(
            transform.position,
            transform.rotation,
            transform.scale
        );

        m_Renderer->SubmitDrawCall(
            entity,
            worldMatrix,
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
