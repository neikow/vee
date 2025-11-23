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

#include "src/engine.h"
#include "src/entities/manager.h"
#include "src/entities/components/component_manager.h"
#include "src/entities/components/transform_component.h"
#include "src/entities/components/velocity_component.h"
#include "src/entities/components/tags/active_camera_tag_component.h"
#include "src/renderer/abstract.h"
#include "src/renderer/vulkan/vertex.h"
#include "src/renderer/vulkan/vulkan_renderer.h"

constexpr uint32_t WIDTH = 1200;
constexpr uint32_t HEIGHT = 600;

int main() {
    const auto g_Renderer = std::reinterpret_pointer_cast<AbstractRenderer>(
        std::make_shared<Vulkan::Renderer>()
    );

    const auto g_Engine = std::make_shared<Engine>(
        g_Renderer
    );

    const EntityID camera = g_Engine->m_EntityManager->CreateEntity();
    const EntityID vikingRoom1 = g_Engine->m_EntityManager->CreateEntity();
    const EntityID vikingRoom2 = g_Engine->m_EntityManager->CreateEntity();

    g_Engine->m_ComponentManager->AddComponent<TransformComponent>(
        vikingRoom1,
        TransformComponent{
            glm::vec3(-1.0f, 0.0f, 0.0f),
            glm::quat(1, glm::radians(-90.0f), 0, 0),
            1.0f
        }
    );

    g_Engine->m_ComponentManager->AddComponent<VelocityComponent>(
        vikingRoom1,
        VelocityComponent{
            glm::vec3(),
            glm::vec3(0.0f, 0.0f, glm::radians(5.0f))
        }
    );

    g_Engine->m_ComponentManager->AddComponent<RenderableComponent>(
        vikingRoom1,
        RenderableComponent{
            0,
            0
        }
    );

    g_Engine->m_ComponentManager->AddComponent<TransformComponent>(
        vikingRoom2,
        TransformComponent{
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::quat(1, glm::radians(-90.0f), 0, 0),
            1.0f
        }
    );

    g_Engine->m_ComponentManager->AddComponent<VelocityComponent>(
        vikingRoom2,
        VelocityComponent{
            glm::vec3(),
            glm::vec3(0.0f, 0.0f, glm::radians(-5.0f))
        }
    );

    g_Engine->m_ComponentManager->AddComponent<RenderableComponent>(
        vikingRoom2,
        RenderableComponent{
            0,
            0
        }
    );

    g_Engine->m_ComponentManager->AddComponent<CameraComponent>(
        camera,
        CameraComponent{
            PERSPECTIVE,
            45.0f,
            0.0f,
            0.01f,
            200.0f

        }
    );

    g_Engine->m_ComponentManager->AddComponent<TransformComponent>(
        camera,
        TransformComponent{
            glm::vec3(0.0f, 0.3f, 3.0f),
            glm::quat(1, glm::radians(-25.0f), 0, 0),
            1.0f
        }
    );

    g_Engine->m_ComponentManager->AddComponent<ActiveCameraTagComponent>(
        camera,
        ActiveCameraTagComponent{}
    );

    try {
        g_Engine->Run(WIDTH, HEIGHT);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
