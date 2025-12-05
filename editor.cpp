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
#include "src/editor/systems/editor_camera_system.h"
#include "src/engine/entities/components_system/components/camera_component.h"
#include "src/engine/entities/components_system/tags/editor_camera_tag_component.h"

constexpr uint32_t WIDTH = 1920;
constexpr uint32_t HEIGHT = 1080;

int main() {
    const auto g_Renderer = std::make_shared<Vulkan::RendererWithUi>();

    const auto g_Engine = std::make_shared<Engine>(g_Renderer);

    const auto g_Editor = std::make_shared<Editor>(g_Engine);

    bool hasError = false;
    try {
        // TODO : move this initialization inside the Editor class
        g_Engine->RegisterSystems(
            [&g_Renderer, &g_Editor](
        auto,
        auto systemManager,
        auto componentManager
    ) {
                componentManager->template RegisterComponent<EditorCameraTagComponent>();

                Signature editorCameraSignature;
                editorCameraSignature.set(ComponentTypeHelper<CameraComponent>::ID);
                editorCameraSignature.set(ComponentTypeHelper<TransformComponent>::ID);
                editorCameraSignature.set(ComponentTypeHelper<EditorCameraTagComponent>::ID);
                systemManager->template RegisterSystem<EditorCameraSystem>(
                    std::make_shared<EditorCameraSystem>(
                        g_Renderer,
                        componentManager,
                        g_Editor.get()
                    )
                );
                systemManager->template SetSignature<EditorCameraSystem>(editorCameraSignature);
            }
        );

        // TODO: Make the scene picking available at runtime to remove this line
        g_Editor->LoadScene("../.editor_data/scenes/scene2.scene");

        g_Editor->Run(WIDTH, HEIGHT);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        hasError = true;
    }

    return hasError ? EXIT_FAILURE : EXIT_SUCCESS;
}
