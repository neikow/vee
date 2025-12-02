#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <shaderc/shaderc.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

#include <glm/glm.hpp>

#include <chrono>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "src/editor/editor.h"

#include "src/engine/engine.h"
#include "src/engine/renderer/vulkan/vulkan_renderer.h"
#include "src/editor/renderer/vulkan/vulkan_renderer_with_ui.h"
#include "src/engine/entities/components_system/components/camera_component.h"
#include "src/engine/entities/components_system/tags/editor_camera_tag_component.h"

constexpr uint32_t WIDTH = 1920;
constexpr uint32_t HEIGHT = 1080;

int main() {
    const auto g_Renderer = std::make_shared<Vulkan::RendererWithUi>();

    const auto g_Engine = std::make_shared<Engine>(
        std::static_pointer_cast<AbstractRenderer>(g_Renderer)
    );

    g_Engine->ToggleEditorMode(true);

    const auto g_Editor = std::make_shared<Editor>(g_Engine);

    bool hasError = false;
    try {
        g_Editor->LoadScene("../.editor_data/scenes/scene2.scene");

        Signature signature;
        signature.set(ComponentTypeHelper<CameraComponent>::ID);
        signature.set(ComponentTypeHelper<EditorCameraTagComponent>::ID);
        g_Engine->SetActiveCameraEntityId(
            Utils::Entities::GetFirstEntityWithSignature(
                g_Engine->GetScene()->GetEntityManager()->GetEntitiesWithSignature(signature)
            )
        );

        g_Editor->Run(WIDTH, HEIGHT);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        hasError = true;
    }

    return hasError ? EXIT_FAILURE : EXIT_SUCCESS;
}
