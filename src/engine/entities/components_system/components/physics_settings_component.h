#ifndef VEE_PHYSICS_SETTINGS_COMPONENT_H
#define VEE_PHYSICS_SETTINGS_COMPONENT_H
#include <glm/glm.hpp>

struct PhysicsSettingsComponent final {
    float gravityAcceleration = 9.81f;
    glm::vec3 gravityDirection = glm::vec3(0.0f, 0.0f, -1.0f);
    int solverIterations = 10;
};

#endif
