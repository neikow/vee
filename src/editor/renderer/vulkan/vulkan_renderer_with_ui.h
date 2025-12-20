#ifndef VEE_VULKAN_RENDERER_WITH_UI_H
#define VEE_VULKAN_RENDERER_WITH_UI_H
#include <memory>
#include <vulkan/vulkan_core.h>

#include "imgui.h"
#include "../../../engine/entities/types.h"
#include "../../../engine/renderer/vulkan/vulkan_renderer.h"

namespace Vulkan {
    class Renderer;

    struct PickingRequest {
        VkBuffer buffer;
        VmaAllocation allocation;
        bool isPending;
        glm::ivec2 pos;
        uint32_t frameSubmitted;
        VkFence fence = VK_NULL_HANDLE;
    };

    class RendererWithUi final : public Renderer {
        VkDescriptorPool m_ImguiDescriptorPool = VK_NULL_HANDLE;

        VkExtent2D m_ViewportExtent = {};
        VkRenderPass m_ViewportRenderPass = VK_NULL_HANDLE;
        VkImage m_ViewportImage = VK_NULL_HANDLE;
        VmaAllocation m_ViewportAllocation = VK_NULL_HANDLE;
        VkImageView m_ViewportImageView = VK_NULL_HANDLE;
        VkFramebuffer m_ViewportFramebuffer = VK_NULL_HANDLE;
        VkDescriptorSet m_ViewportDescriptorSet = VK_NULL_HANDLE;

        PickingRequest m_PickingRequest;
        Entities::EntityID m_LastPickedEntityID = Entities::NULL_ENTITY;

        VkImage m_PickingImage = VK_NULL_HANDLE;
        VmaAllocation m_PickingAllocation = VK_NULL_HANDLE;
        VkImageView m_PickingImageView = VK_NULL_HANDLE;
        VkFramebuffer m_PickingFramebuffer = VK_NULL_HANDLE;

        VkRenderPass m_PickingRenderPass = VK_NULL_HANDLE;
        VkPipeline m_PickingPipeline = VK_NULL_HANDLE;

        bool m_ViewportResized = false;

    public:
        explicit RendererWithUi(const std::shared_ptr<Window> &window);

        void Initialize(
            const std::string &appName,
            uint32_t version
        ) override;

        float GetAspectRatio() override;

        void Cleanup() override;

        void RequestEntityIDAt(double normX, double normY);

        [[nodiscard]] bool IsPickingRequestPending() const;

        void ExecutePickingPass(const VkCommandBuffer &cmd) const;

        void RecordPickingCopy(const VkCommandBuffer &cmd) const;

        void UpdatePickingResult();

        void BuildRenderGraph() override;

        [[nodiscard]] VkDescriptorSet GetViewportDescriptorSet() const;

        void UpdateViewportSize(uint32_t width, uint32_t height);

        [[nodiscard]] Entities::EntityID GetLastPickedID() const;

        std::shared_ptr<Window> GetWindow();

    private:
        void AddResizeCallbacks() override;

        void InitImgui();

        void CreateViewportRenderPass();

        void CreateGraphicsResources() override;

        void CreateViewportResources();

        void CleanupViewportResources() const;

        void CreatePickingRenderPass();

        void CreatePickingPipeline();

        void RenderUI(const VkCommandBuffer &cmd) const;

        void CreatePickingResources();

        void CleanupPickingResources();

        void PrepareForRendering() override;
    };
}


#endif //VEE_VULKAN_RENDERER_WITH_UI_H
