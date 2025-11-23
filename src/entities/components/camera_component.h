#ifndef GAME_ENGINE_CAMERA_COMPONENT_H
#define GAME_ENGINE_CAMERA_COMPONENT_H
#include <glm/glm.hpp>

enum ProjectionType {
    PERSPECTIVE,
    ORTHOGRAPHIC
};

struct CameraComponent {
    ProjectionType projection = PERSPECTIVE;

    // Perspective Projection
    float fieldOfView = 45.0f;
    float aspectRatio = 16.0f / 9.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;

    // Orthographic Projection
    float orthoScale = 10.0f;

    // Cached variables (per frame)
    // Cached
    glm::mat4 viewMatrix{};
    // Cached
    glm::mat4 projectionMatrix{};
};


#endif //GAME_ENGINE_CAMERA_COMPONENT_H
