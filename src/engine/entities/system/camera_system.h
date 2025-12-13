#ifndef GAME_ENGINE_CAMERA_SYSTEM_H
#define GAME_ENGINE_CAMERA_SYSTEM_H
#include "system.h"
#include "../../logging/logger.h"

#include "../../renderer/abstract.h"
#include "../../utils/macros/log_macros.h"
#include "../components_system/component_manager.h"
#include "../components_system/components/local_transform_component.h"
#include "../components_system/components/camera_component.h"
#include "../components_system/components/parent_component.h"
#include "../components_system/tags/editor_camera_tag_component.h"


class CameraSystem : public SystemBase {
    using SystemBase::SystemBase;

protected:
    std::shared_ptr<AbstractRenderer> m_Renderer;

public:
    CameraSystem(
        const std::shared_ptr<AbstractRenderer> &renderer,
        const std::shared_ptr<ComponentManager> &componentManager
    ) : SystemBase(componentManager), m_Renderer(renderer) {
    }

    [[nodiscard]] EntityID GetActiveCameraId() const {
        std::optional<EntityID> activeCameraId;

        if (!m_Entities.empty()) {
            activeCameraId = *m_Entities.begin();
        }

        if (!activeCameraId.has_value()) {
            LOG_ERROR(
                "No active camera found in CameraSystem. "
                "You should set the Camera's ActiveCameraTagComponent for it to be picked up."
            );
            return NULL_ENTITY;
        }

        return activeCameraId.value();
    }

    virtual glm::mat4 ComputeViewMatrix(LocalTransformComponent &transform) {
        const glm::mat4 M_world = glm::translate(glm::mat4_cast(transform.rotation), transform.position);

        return glm::inverse(M_world);
    }

    [[nodiscard]] virtual glm::mat4 ComputeProjectionMatrix(const CameraComponent &cameraComponent) const {
        const auto aspectRatio = cameraComponent.aspectRatio == 0.0f
                                     ? m_Renderer->GetAspectRatio()
                                     : cameraComponent.aspectRatio;

        if (cameraComponent.projection == PERSPECTIVE) {
            return glm::perspective(
                glm::radians(cameraComponent.fieldOfView),
                aspectRatio,
                cameraComponent.nearPlane,
                cameraComponent.farPlane
            );
        }

        return glm::ortho(
            -cameraComponent.orthoScale, cameraComponent.orthoScale,
            -cameraComponent.orthoScale / cameraComponent.aspectRatio,
            cameraComponent.orthoScale / cameraComponent.aspectRatio,
            cameraComponent.nearPlane, cameraComponent.farPlane
        );
    }

    virtual void UpdateCamera(CameraComponent &cameraComponent) const {
    }

    void Update(float dt) override {
        const auto activeCameraId = GetActiveCameraId();
        if (activeCameraId == NULL_ENTITY) {
            return;
        }

        // TODO: Use LocalToWorldComponent to allow nested cameras
        if (m_ComponentManager->HasComponent<ParentComponent>(activeCameraId)) {
            LOG_WARN("Not implemented : CameraSystem does not support ParentComponent on camera entities yet.");
        }

        auto &transform = m_ComponentManager->GetComponent<LocalTransformComponent>(activeCameraId);
        auto &cameraComponent = m_ComponentManager->GetComponent<CameraComponent>(activeCameraId);

        UpdateCamera(cameraComponent);

        cameraComponent.viewMatrix = ComputeViewMatrix(transform);
        cameraComponent.projectionMatrix = ComputeProjectionMatrix(cameraComponent);
    }
};


#endif //GAME_ENGINE_CAMERA_SYSTEM_H
