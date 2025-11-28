#ifndef VEE_VULKAN_RENDERER_WITH_UI_H
#define VEE_VULKAN_RENDERER_WITH_UI_H
#include <memory>
#include <vulkan/vulkan_core.h>

#include "imgui.h"
#include "../../../engine/renderer/vulkan/vulkan_renderer.h"

namespace Vulkan {
    class Renderer;

    class RendererWithUi final : public Renderer {
        ImDrawData *m_DrawData = nullptr;
        VkDescriptorPool m_ImguiDescriptorPool = VK_NULL_HANDLE;

    public:
        RendererWithUi() = default;

        void Initialize(int width, int height, const std::string &appName, uint32_t version) override;

        void Draw() override;

        void SubmitUIDrawData(ImDrawData *drawData) override;

        void Cleanup() override;

    private:
        void InitImgui();
    };
}


#endif //VEE_VULKAN_RENDERER_WITH_UI_H
