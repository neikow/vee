#ifndef GAME_ENGINE_MATH_H
#define GAME_ENGINE_MATH_H
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace Utils::Math {
    glm::mat4x4 CalculateWorldMatrix(
        glm::vec3 position,
        glm::quat rotation,
        glm::vec3 scale
    );
}


#endif //GAME_ENGINE_MATH_H
