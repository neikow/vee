#ifndef VEE_VULKAN_RENDERER_WITH_UI_H
#define VEE_VULKAN_RENDERER_WITH_UI_H
#include <memory>
#include <vulkan/vulkan_core.h>

#include "imgui.h"
#include "../../../engine/entities/types.h"
#include "../../../engine/renderer/vulkan/vulkan_renderer.h"

namespace Vulkan {
    class Renderer;

    class RendererWithUi final : public Renderer {
        VkDescriptorPool m_ImguiDescriptorPool = VK_NULL_HANDLE;

        VkImage m_PickingImage = VK_NULL_HANDLE;
        VkDeviceMemory m_PickingMemory = VK_NULL_HANDLE;
        VkImageView m_PickingImageView = VK_NULL_HANDLE;
        VkFramebuffer m_PickingFramebuffer = VK_NULL_HANDLE;

        VkRenderPass m_PickingRenderPass = VK_NULL_HANDLE;
        VkPipeline m_PickingPipeline = VK_NULL_HANDLE;

    public:
        RendererWithUi() = default;

        void Initialize(int width, int height, const std::string &appName, uint32_t version) override;

        void Cleanup() override;

        Entities::EntityID GetEntityIDAt(double norm_x, double norm_y) const;

    private:
        void InitImgui();

        void CreateGraphicsResources() override;

        void CreatePickingRenderPass();

        void CreatePickingPipeline();

        void RenderToScreen(const VkCommandBuffer &cmd) override;

        void CreatePickingResources();

        void CleanupPickingResources() const;

        void PrepareForRendering() override;
    };
}


#endif //VEE_VULKAN_RENDERER_WITH_UI_H
