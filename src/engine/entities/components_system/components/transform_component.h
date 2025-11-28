#ifndef GAME_ENGINE_TRANSFORM_COMPONENT_H
#define GAME_ENGINE_TRANSFORM_COMPONENT_H

#include "glm/gtc/quaternion.hpp"

struct TransformComponent final {
    glm::vec3 position;
    glm::quat rotation = glm::quat(1, 0, 0, 0);
    glm::vec3 scale = {1.0f, 1.0f, 1.0f};
};

#endif //GAME_ENGINE_TRANSFORM_COMPONENT_H
