#include "math_utils.h"

glm::mat4x4 Utils::Math::CalculateWorldMatrix(
    const glm::vec3 position,
    const glm::quat rotation,
    const float scale
) {
    const auto translationMatrix = glm::translate(
        glm::mat4(1.0f),
        position
    );
    const auto rotationMatrix = glm::mat4_cast(
        rotation
    );
    const auto scaleMatrix = glm::scale(
        glm::mat4(1.0f),
        glm::vec3(scale)
    );

    return translationMatrix * rotationMatrix * scaleMatrix;
}
