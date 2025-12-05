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
#include "src/engine/entities/components_system/components/transform_component.h"
#include "src/engine/entities/components_system/components/velocity_component.h"
#include "src/engine/entities/components_system/tags/active_camera_tag_component.h"
#include "src/engine/io/input_system.h"
#include "src/engine/renderer/abstract.h"
#include "src/engine/renderer/vulkan/vertex_utils.h"
#include "src/engine/renderer/vulkan/vulkan_renderer.h"

constexpr uint32_t WIDTH = 1200;
constexpr uint32_t HEIGHT = 600;

int main() {
    const auto g_Renderer = std::make_shared<Vulkan::Renderer>();

    const auto g_Engine = std::make_shared<Engine>(std::static_pointer_cast<AbstractRenderer>(g_Renderer));

    g_Engine->LoadScene("../.editor_data/scenes/scene2.scene");

    bool hasError = false;

    try {
        g_Engine->Initialize(WIDTH, HEIGHT, "Engine", VK_MAKE_VERSION(1, 0, 0));

        Signature signature;
        signature.set(ComponentTypeHelper<CameraComponent>::ID);
        signature.set(ComponentTypeHelper<TransformComponent>::ID);
        signature.set(ComponentTypeHelper<ActiveCameraTagComponent>::ID);

        const auto cameraId = Utils::Entities::GetFirstEntityWithSignature(
            g_Engine->GetScene()->GetEntityManager()->GetEntitiesWithSignature(signature)
        );

        std::cout << cameraId << std::endl;

        g_Engine->SetActiveCameraEntityId(cameraId);

        auto startTime = std::chrono::high_resolution_clock::now();

        while (!g_Engine->ShouldQuit() && !g_Engine->GetRenderer()->ShouldClose()) {
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
