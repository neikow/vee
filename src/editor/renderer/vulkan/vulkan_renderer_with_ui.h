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

        VkExtent2D m_ViewportExtent = {};
        VkImage m_ViewportImage = VK_NULL_HANDLE;
        VkDeviceMemory m_ViewportMemory = VK_NULL_HANDLE;
        VkImageView m_ViewportImageView = VK_NULL_HANDLE;
        VkFramebuffer m_ViewportFramebuffer = VK_NULL_HANDLE;
        VkDescriptorSet m_ViewportDescriptorSet = VK_NULL_HANDLE;

        VkImage m_PickingImage = VK_NULL_HANDLE;
        VkDeviceMemory m_PickingMemory = VK_NULL_HANDLE;
        VkImageView m_PickingImageView = VK_NULL_HANDLE;
        VkFramebuffer m_PickingFramebuffer = VK_NULL_HANDLE;

        VkRenderPass m_PickingRenderPass = VK_NULL_HANDLE;
        VkPipeline m_PickingPipeline = VK_NULL_HANDLE;

    public:
        RendererWithUi() = default;

        void Initialize(int width, int height, const std::string &appName, uint32_t version) override;

        float GetAspectRatio() override;

        void Cleanup() override;

        Entities::EntityID GetEntityIDAt(double norm_x, double norm_y) const;

        [[nodiscard]] VkDescriptorSet GetViewportDescriptorSet() const;

        void UpdateViewportSize(uint32_t width, uint32_t height);

    private:
        void InitImgui();

        void CreateGraphicsResources() override;

        void CreateViewportResources();

        void CleanupViewportResources() const;

        void CreatePickingRenderPass();

        void CreatePickingPipeline();

        void RenderToScreen(const VkCommandBuffer &cmd) override;

        void CreatePickingResources();

        void CleanupPickingResources() const;

        void PrepareForRendering() override;
    };
}


#endif //VEE_VULKAN_RENDERER_WITH_UI_H
