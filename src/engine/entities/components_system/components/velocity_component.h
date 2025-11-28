#ifndef GAME_ENGINE_VELOCITY_COMPONENT_H
#define GAME_ENGINE_VELOCITY_COMPONENT_H
#include <glm/glm.hpp>

struct VelocityComponent final {
    glm::vec3 linearVelocity;
    glm::vec3 angularVelocity;
};


#endif //GAME_ENGINE_VELOCITY_COMPONENT_H
