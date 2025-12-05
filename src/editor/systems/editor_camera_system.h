#ifndef GAME_ENGINE_EDITOR_CAMERA_SYSTEM_H
#define GAME_ENGINE_EDITOR_CAMERA_SYSTEM_H
#include "../editor.h"
#include "../../engine/entities/system/camera_system.h"
#include "../../engine/io/input_system.h"

#include "../../engine/renderer/abstract.h"
#include "../../engine/entities/components_system/components/camera_component.h"


class EditorCameraSystem final : public CameraSystem {
    const Editor *m_Editor;

public:
    EditorCameraSystem(
        const std::shared_ptr<AbstractRenderer> &renderer,
        const std::shared_ptr<ComponentManager> &componentManager,
        const Editor *editor
    ) : CameraSystem(renderer, componentManager), m_Editor(editor) {
    }

    void UpdateCamera(CameraComponent &cameraComponent) const override {
        const auto scrollDelta = InputSystem::GetScrollDeltaY();
        if (scrollDelta == 0.0f) return;

        const auto editorState = m_Editor->GetEditorState();
        if (!editorState.isViewportFocused || !editorState.isViewportHovered) return;

        const auto zoomSpeed = m_Editor->GetEditorSettings().zoomSpeed;

        if (cameraComponent.projection == PERSPECTIVE) {
            const float newFOV = cameraComponent.fieldOfView - (scrollDelta * zoomSpeed);

            cameraComponent.fieldOfView = glm::clamp(newFOV, 1.0f, 120.0f);
        } else if (cameraComponent.projection == ORTHOGRAPHIC) {
            const float newOrthoScale = cameraComponent.orthoScale - (scrollDelta * zoomSpeed * 0.1f);

            cameraComponent.orthoScale = glm::clamp(newOrthoScale, 0.1f, 1000.0f);
        } else {
            throw std::runtime_error("Unsupported projection type");
        }
    }

private:
    using CameraSystem::CameraSystem;
};


#endif //GAME_ENGINE_EDITOR_CAMERA_SYSTEM_H
