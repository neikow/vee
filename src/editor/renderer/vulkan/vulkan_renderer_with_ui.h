#ifndef VEE_VULKAN_RENDERER_WITH_UI_H
#define VEE_VULKAN_RENDERER_WITH_UI_H
#include <memory>
#include <vulkan/vulkan_core.h>

#include "imgui.h"
#include "../../../engine/renderer/abstract.h"
#include "../../../engine/renderer/vulkan/vulkan_renderer.h"

namespace Vulkan {
    class Renderer;

    class RendererWithUi final : public AbstractRenderer {
        std::shared_ptr<Renderer> m_CoreRenderer;
        ImDrawData *m_DrawData = nullptr;
        VkDescriptorPool m_ImguiDescriptorPool = VK_NULL_HANDLE;

    public:
        explicit RendererWithUi(const std::shared_ptr<Renderer> &renderer)
            : m_CoreRenderer(renderer) {
        }

        void Initialize(int width, int height, const std::string &appName, uint32_t version) override;

        void BeginFrame() override;

        void EndFrame() override;;

        void SubmitUIDrawData(ImDrawData *drawData) override;

        void UpdateCameraMatrix(const glm::mat4x4 &viewMatrix, const glm::mat4x4 &projectionMatrix) override;;

        void SubmitDrawCall(const glm::mat4x4 &worldMatrix, std::uint32_t meshId, std::uint32_t textureId) override;

        void Cleanup() override;

        bool ShouldClose() override;

        void WaitIdle() override;

        float GetAspectRatio() override;

    private:
        void InitImgui();
    };
}


#endif //VEE_VULKAN_RENDERER_WITH_UI_H
