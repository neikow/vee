#include "display_system.h"

#include "../../utils/math_utils.h"
#include "../components/transform_component.h"
#include "../components/camera_component.h"
#include "../components/renderable_component.h"
#include "../components/component_manager.h"
#include "../components/tags/active_camera_tag_component.h"

std::optional<EntityID> GetActiveCameraEntityId(
    const std::shared_ptr<EntityManager> &entityManager
) {
    Signature cameraSignature;
    cameraSignature.set(ComponentTypeHelper<CameraComponent>::ID);
    cameraSignature.set(ComponentTypeHelper<ActiveCameraTagComponent>::ID);

    const auto cameraEntities = entityManager->GetEntitiesWithSignature(cameraSignature);
    for (const auto &entityId: cameraEntities) {
        return entityId;
    }

    return std::nullopt;
}

bool DisplaySystem::PrepareCamera() const {
    const auto result = GetActiveCameraEntityId(m_EntityManager);

    if (!result.has_value()) {
        std::cerr << "[WARN] No active camera entity found." << std::endl;
        return false;
    }

    const auto currentCameraEntityId = result.value();
    const auto &cameraComponent = m_ComponentManager->GetComponent<CameraComponent>(currentCameraEntityId);

    m_Renderer->UpdateCameraMatrix(
        cameraComponent.viewMatrix,
        cameraComponent.projectionMatrix
    );

    return true;
}

void DisplaySystem::Render(float interpolationFactor) const {
    m_Renderer->BeginFrame();

    // Prepare Camera
    if (!PrepareCamera()) {
        m_Renderer->EndFrame();
        return;
    };

    // Prepare Light
    // TODO: Implement lights

    // Prepare and submit draw calls for each entity
    for (const auto &entity: m_Entities) {
        const auto &transform = m_ComponentManager->GetComponent<TransformComponent>(entity);
        const auto &renderable = m_ComponentManager->GetComponent<RenderableComponent>(entity);

        glm::mat4x4 worldMatrix = Utils::Math::CalculateWorldMatrix(
            transform.position,
            transform.rotation,
            transform.scale
        );

        m_Renderer->SubmitDrawCall(
            worldMatrix,
            renderable.meshId,
            renderable.textureId
        );
    }

    m_Renderer->EndFrame();
}
