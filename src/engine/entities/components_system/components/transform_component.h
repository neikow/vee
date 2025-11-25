#ifndef GAME_ENGINE_TRANSFORM_COMPONENT_H
#define GAME_ENGINE_TRANSFORM_COMPONENT_H

#include "glm/gtc/quaternion.hpp"

struct TransformComponent final {
    glm::vec3 position;
    glm::quat rotation = glm::quat(1, 0, 0, 0);
    float scale = 1.0f;

    explicit TransformComponent(
        const glm::vec3 &position,
        const glm::quat &rotation,
        const float scale
    )
        : position(position), rotation(rotation), scale(scale) {
    }
};

#endif //GAME_ENGINE_TRANSFORM_COMPONENT_H
