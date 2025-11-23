#ifndef GAME_ENGINE_CAMERA_SYSTEM_H
#define GAME_ENGINE_CAMERA_SYSTEM_H
#include "system.h"

#include "../../renderer/abstract.h"
#include "../components/component_manager.h"
#include "../components/transform_component.h"
#include "../components/camera_component.h"


class CameraSystem final : public SystemBase {
    using SystemBase::SystemBase;

    std::shared_ptr<AbstractRenderer> m_Renderer;

public:
    CameraSystem(
        const std::shared_ptr<AbstractRenderer> &renderer,
        const std::shared_ptr<ComponentManager> &componentManager
    ) : SystemBase(componentManager), m_Renderer(renderer) {
    }

    void Update(float dt) override {
        std::optional<EntityID> activeCameraId;

        if (!m_Entities.empty()) {
            activeCameraId = *m_Entities.begin();
        }

        if (!activeCameraId.has_value()) {
            std::cerr <<
                    "No active camera found in CameraSystem. "
                    "You should set the Camera's ActiveCameraTagComponent for it to be picked up."
                    << std::endl;
            return;
        }


        const auto &transform = m_ComponentManager->GetComponent<TransformComponent>(activeCameraId.value());
        auto &cameraComponent = m_ComponentManager->GetComponent<CameraComponent>(activeCameraId.value());
        const glm::mat4 M_world = glm::translate(glm::mat4_cast(transform.rotation), transform.position);

        cameraComponent.viewMatrix = glm::inverse(M_world);

        const auto aspectRatio = cameraComponent.aspectRatio == 0.0f
                                     ? m_Renderer->GetAspectRatio()
                                     : cameraComponent.aspectRatio;

        if (cameraComponent.projection == PERSPECTIVE) {
            cameraComponent.projectionMatrix = glm::perspective(
                glm::radians(cameraComponent.fieldOfView),
                aspectRatio,
                cameraComponent.nearPlane,
                cameraComponent.farPlane
            );
        } else {
            cameraComponent.projectionMatrix = glm::ortho(
                -cameraComponent.orthoScale, cameraComponent.orthoScale,
                -cameraComponent.orthoScale / cameraComponent.aspectRatio,
                cameraComponent.orthoScale / cameraComponent.aspectRatio,
                cameraComponent.nearPlane, cameraComponent.farPlane
            );
        }
    };
};


#endif //GAME_ENGINE_CAMERA_SYSTEM_H
