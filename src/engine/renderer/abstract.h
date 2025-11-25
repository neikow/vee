#ifndef GAME_ENGINE_ABSTRACT_H
#define GAME_ENGINE_ABSTRACT_H
#include <cstdint>
#include <glm/mat4x4.hpp>

#include "imgui.h"


class AbstractRenderer {
public:
    AbstractRenderer() = default;

    virtual void Initialize(int width, int height, const std::string &appName, uint32_t version) = 0;

    virtual void Draw() = 0;

    virtual void UpdateCameraMatrix(
        const glm::mat4x4 &viewMatrix,
        const glm::mat4x4 &projectionMatrix
    ) = 0;

    virtual void SubmitDrawCall(
        const glm::mat4x4 &worldMatrix,
        std::uint32_t meshId,
        std::uint32_t textureId
    ) = 0;

    virtual void Cleanup() = 0;

    virtual bool ShouldClose() = 0;

    virtual void WaitIdle() = 0;

    virtual float GetAspectRatio() = 0;

    virtual void SubmitUIDrawData(ImDrawData *drawData) = 0;

    virtual ~AbstractRenderer() = default;
};


#endif //GAME_ENGINE_ABSTRACT_H
