#ifndef VEE_PLAYER_CONTROLLER_COMPONENT_H
#define VEE_PLAYER_CONTROLLER_COMPONENT_H
#include "../../../io/input_system.h"

struct PlayerControllerComponent final {
    float movementSpeed = 5.0f;

    glm::vec3 forwardDirection = glm::vec3(0, 1, 0);

    Key moveForwardKey = GLFW_KEY_W;
    Key moveBackwardKey = GLFW_KEY_S;
    Key moveLeftKey = GLFW_KEY_A;
    Key moveRightKey = GLFW_KEY_D;
};

#endif //VEE_PLAYER_CONTROLLER_COMPONENT_H
