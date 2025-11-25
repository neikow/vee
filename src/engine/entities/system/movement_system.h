#ifndef GAME_ENGINE_MOVEMENT_SYSTEM_H
#define GAME_ENGINE_MOVEMENT_SYSTEM_H

#include "system.h"
#include "../components_system/component_manager.h"
#include "../components_system/components/transform_component.h"
#include "../components_system/components/velocity_component.h"


struct VelocityComponent;

class MovementSystem final : public SystemBase {
public:
    using SystemBase::SystemBase;

    void Update(const float dt) override {
        for (const auto entity: m_Entities) {
            auto &transform = m_ComponentManager->GetComponent<TransformComponent>(entity);
            const auto &velocity = m_ComponentManager->GetComponent<VelocityComponent>(entity);

            transform.position += velocity.linearVelocity * dt;

            const float angularRate = glm::length(velocity.angularVelocity);

            if (angularRate > 0.0001f) {
                float angle = angularRate * dt;
                glm::vec3 axis = velocity.angularVelocity / angularRate;
                glm::quat deltaRotation = glm::angleAxis(angle, axis);
                transform.rotation = transform.rotation * deltaRotation;
                transform.rotation = glm::normalize(transform.rotation);
            }
        }
    }
};


#endif //GAME_ENGINE_MOVEMENT_SYSTEM_H
