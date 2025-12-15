#include "player_controller_system.h"
#include <glm/glm.hpp>
#include "../components_system/components/player_controller_component.h"
#include "../components_system/components/velocity_component.h"
#include "../../io/input_system.h"
#include "../components_system/component_manager.h"

PlayerControllerSystem::PlayerControllerSystem(
    const std::shared_ptr<ComponentManager> &componentManager
) : SystemBase(componentManager) {
}

void PlayerControllerSystem::Update(float dt) {
    for (const auto &entity: m_Entities) {
        auto &velocity = m_ComponentManager->GetComponent<VelocityComponent>(entity);
        const auto &playerController = m_ComponentManager->GetComponent<PlayerControllerComponent>(entity);

        glm::vec3 direction(0.0f);

        if (InputSystem::IsKeyPressed(playerController.moveForwardKey)) {
            direction += playerController.forwardDirection;
        }
        if (InputSystem::IsKeyPressed(playerController.moveBackwardKey)) {
            direction -= playerController.forwardDirection;
        }
        if (InputSystem::IsKeyPressed(playerController.moveLeftKey)) {
            direction -= glm::normalize(glm::cross(playerController.forwardDirection, glm::vec3(0, 0, 1)));
        }
        if (InputSystem::IsKeyPressed(playerController.moveRightKey)) {
            direction += glm::normalize(glm::cross(playerController.forwardDirection, glm::vec3(0, 0, 1)));
        }

        if (glm::length(direction) > 0.0f) {
            direction = glm::normalize(direction);
        }

        velocity.linearVelocity = direction * playerController.movementSpeed;
    }
}
