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

#include "src/editor/renderer/vulkan/vulkan_renderer_with_ui.h"
#include "src/engine/engine.h"
#include "src/engine/models/mesh_manager/vulkan_mesh_manager.h"
#include "src/engine/renderer/vulkan/vulkan_renderer.h"


constexpr uint32_t WIDTH = 1920;
constexpr uint32_t HEIGHT = 1080;

int main() {
    const auto g_MeshManager = std::make_shared<Vulkan::MeshManager>();
    const auto g_TextureManager = std::make_shared<Vulkan::TextureManager>();
    const auto g_Renderer = std::make_shared<Vulkan::RendererWithUi>(
        std::make_shared<Vulkan::Renderer>(
            g_MeshManager,
            g_TextureManager
        )
    );

    const auto g_SystemManager = std::make_shared<SystemManager>();

    const auto g_EntityManager = std::make_shared<EntityManager>();

    const auto g_ComponentManager = std::make_shared<ComponentManager>(g_SystemManager, g_EntityManager);

    const auto g_Engine = std::make_shared<Engine>(
        std::reinterpret_pointer_cast<AbstractRenderer>(g_Renderer),
        g_SystemManager,
        g_EntityManager,
        g_ComponentManager
    );

    const EntityID camera = g_Engine->CreateEntity();
    const EntityID vikingRoom = g_Engine->CreateEntity();

    const auto vikingRoomMesh = g_MeshManager->LoadModel("../assets/models/viking_room.obj");
    const auto vikingRoomTexture = g_TextureManager->LoadTexture("../assets/textures/viking_room.png");

    g_ComponentManager->AddComponent<TransformComponent>(
        vikingRoom,
        TransformComponent{
            glm::vec3(-1.0f, 0.0f, 0.0f),
            glm::quat(1, glm::radians(-90.0f), 0, 0),
            1.0f
        }
    );

    g_ComponentManager->AddComponent<VelocityComponent>(
        vikingRoom,
        VelocityComponent{
            glm::vec3(),
            glm::vec3(0.0f, 0.0f, glm::radians(15.0f))
        }
    );

    g_ComponentManager->AddComponent<RenderableComponent>(
        vikingRoom,
        RenderableComponent{
            vikingRoomMesh,
            vikingRoomTexture
        }
    );

    g_ComponentManager->AddComponent<CameraComponent>(
        camera,
        CameraComponent{
            PERSPECTIVE,
            45.0f,
            0.0f,
            0.01f,
            200.0f

        }
    );

    g_ComponentManager->AddComponent<TransformComponent>(
        camera,
        TransformComponent{
            glm::vec3(0.0f, 0.3f, 3.0f),
            glm::quat(1, glm::radians(-25.0f), 0, 0),
            1.0f
        }
    );

    g_ComponentManager->AddComponent<ActiveCameraTagComponent>(
        camera,
        ActiveCameraTagComponent{}
    );

    auto g_Editor = std::make_shared<Editor>(g_Engine);

    bool hasError = false;
    try {
        g_Editor->Run(WIDTH, HEIGHT);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        hasError = true;
    }

    return hasError ? EXIT_FAILURE : EXIT_SUCCESS;
}
