#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <shaderc/shaderc.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stb_image.h>

#include <chrono>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "src/engine/engine.h"

#include "src/editor/renderer/vulkan/vulkan_renderer_with_ui.h"
#include "src/engine/entities/manager.h"
#include "src/engine/entities/components_system/component_manager.h"
#include "src/engine/entities/components_system/components/camera_component.h"
#include "src/engine/entities/components_system/components/renderable_component.h"
#include "src/engine/entities/components_system/components/local_transform_component.h"
#include "src/engine/entities/components_system/components/velocity_component.h"
#include "src/engine/entities/components_system/tags/active_camera_tag_component.h"
#include "src/engine/entities/system/camera_system.h"
#include "src/engine/io/input_system.h"
#include "src/engine/renderer/abstract.h"
#include "src/engine/renderer/vulkan/vertex_utils.h"
#include "src/engine/renderer/vulkan/vulkan_renderer.h"

constexpr uint32_t WIDTH = 1200;
constexpr uint32_t HEIGHT = 600;

int main() {
    const auto g_Window = std::make_shared<Window>(WIDTH, HEIGHT, "Engine Window");

    const auto g_Renderer = std::make_shared<Vulkan::Renderer>(g_Window);

    const auto g_Engine = std::make_shared<Engine>(std::static_pointer_cast<AbstractRenderer>(g_Renderer));

    bool hasError = false;

    try {
        g_Engine->RegisterSystems([&g_Renderer](auto, auto &systemManager, auto &componentManager) {
            Signature activeCameraSignature;
            activeCameraSignature.set(ComponentTypeHelper<CameraComponent>::ID);
            activeCameraSignature.set(ComponentTypeHelper<LocalTransformComponent>::ID);
            activeCameraSignature.set(ComponentTypeHelper<ActiveCameraTagComponent>::ID);
            systemManager->template RegisterSystem<CameraSystem>(
                std::make_shared<CameraSystem>(
                    g_Renderer,
                    componentManager
                )
            );
            systemManager->template SetSignature<CameraSystem>(activeCameraSignature);
        });

        g_Engine->LoadScene("../.editor_data/scenes/scene1.scene");

        g_Engine->Initialize("Engine", VK_MAKE_VERSION(1, 0, 0));

        Signature signature;
        signature.set(ComponentTypeHelper<CameraComponent>::ID);
        signature.set(ComponentTypeHelper<LocalTransformComponent>::ID);
        signature.set(ComponentTypeHelper<ActiveCameraTagComponent>::ID);

        const auto cameraId = Utils::Entities::GetFirstEntityWithSignature(
            g_Engine->GetScene()->GetEntityManager()->GetEntitiesWithSignature(signature)
        );

        g_Engine->SetActiveCameraEntityId(cameraId);

        auto startTime = std::chrono::high_resolution_clock::now();

        while (!g_Engine->ShouldQuit() && !g_Window->ShouldClose()) {
            const auto deltaTime = std::chrono::duration<float>(
                std::chrono::high_resolution_clock::now() - startTime
            ).count();
            startTime = std::chrono::high_resolution_clock::now();

            glfwPollEvents();
            g_Engine->UpdateSystems(deltaTime);
            g_Engine->PrepareForRendering();
            g_Engine->GetRenderer()->Draw();
            InputSystem::UpdateEndOfFrame();
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        hasError = true;
    }
    g_Engine->Shutdown();

    return hasError ? EXIT_FAILURE : EXIT_SUCCESS;
}
