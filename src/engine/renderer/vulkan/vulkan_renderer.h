#ifndef GAME_ENGINE_VULKAN_RENDERER_H
#define GAME_ENGINE_VULKAN_RENDERER_H

#define GLFW_INCLUDE_VULKAN
#include <vk_mem_alloc.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#include "pipeline_builder.h"
#include "shader_module_cache.h"
#include "swapchain.h"
#include "../abstract.h"
#include "../window.h"
#include "../../models/mesh_manager/mesh_manager.h"
#include "../../models/texture_manager/vulkan_texture_manager.h"
#include "../../utils/renderer/deletion_queue.h"


namespace Vulkan {
    class VulkanDevice;

    constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    constexpr uint64_t MAX_GEOMETRY_BUFFER_SIZE = 256 * 1024 * 1024; // 256 MB
    struct DrawCall {
        glm::mat4 worldMatrix;
        Entities::EntityID entityId;
        std::uint32_t meshId;
        std::uint32_t textureId;
    };

    class Renderer : public AbstractRenderer {
        friend class TextureManager;
        friend class RendererWithUi;

        std::unique_ptr<DeletionQueue> m_ResourceDeletionQueue;
        std::uint32_t m_TotalFramesRendered = 0;

        std::shared_ptr<Window> m_Window;
        bool m_FramebufferResized = false;

        std::shared_ptr<VulkanDevice> m_Device;
        std::shared_ptr<Swapchain> m_Swapchain;
        std::shared_ptr<ShaderModuleCache> m_ShaderModuleCache;

        VkRenderPass m_MainRenderPass = VK_NULL_HANDLE;

        VkDeviceSize m_PaddedUniformBufferSize = 0;
        VkDescriptorSetLayout m_BindlessDescriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_DynamicDescriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorSet m_BindlessDescriptorSet = VK_NULL_HANDLE;
        VkDescriptorSet m_DynamicDescriptorSet = VK_NULL_HANDLE;
        VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;

        std::shared_ptr<PipelineCache> m_PipelineCache;
        VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_GraphicsPipeline = VK_NULL_HANDLE;

        std::vector<VkCommandBuffer> m_CommandBuffers;

        VkSampler m_TextureSampler = VK_NULL_HANDLE;

        std::vector<DrawCall> m_DrawQueue;

        VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
        VmaAllocation m_VertexAllocation = VK_NULL_HANDLE;
        VkBuffer m_IndexBuffer = VK_NULL_HANDLE;
        VmaAllocation m_IndexAllocation = VK_NULL_HANDLE;

        VkBuffer m_UniformBuffer = VK_NULL_HANDLE;
        VmaAllocation m_UniformBufferAllocation = VK_NULL_HANDLE;
        void *m_UniformBufferMapped = nullptr;

        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;

        VkImage m_DefaultTextureImage = VK_NULL_HANDLE;
        VmaAllocation m_DefaultTextureAllocation = VK_NULL_HANDLE;
        VkImageView m_DefaultTextureImageView = VK_NULL_HANDLE;
        VkFramebuffer m_DefaultTextureFramebuffer = VK_NULL_HANDLE;

        uint32_t m_CurrentFrameIndex = 0;
        uint32_t m_ImageIndex = 0;
        uint64_t m_CurrentVertexBufferSize = 0;
        uint64_t m_CurrentIndexBufferSize = 0;
        unsigned long m_ResizeCallbackHandle = 0;

    public:
        explicit Renderer(
            const std::shared_ptr<Window> &window
        );

        void Initialize(
            const std::string &appName,
            uint32_t version
        ) override;

        void RecordDrawQueue(
            const VkCommandBuffer &commandBuffer
        );

        void CalculatePaddedUboSize();

        void RenderScene(
            const VkCommandBuffer &commandBuffer,
            const VkFramebuffer &framebuffer,
            VkExtent2D extent
        ) const;

        virtual void RenderToScreen(
            const VkCommandBuffer &cmd
        );

        void Draw() override;

        void UpdateCameraMatrix(
            const glm::mat4x4 &viewMatrix,
            const glm::mat4x4 &projectionMatrix
        ) override;

        void SubmitDrawCall(
            std::uint32_t entityId,
            const glm::mat4x4 &worldMatrix,
            uint32_t meshId,
            uint32_t textureId
        ) override;

        void Cleanup() override;

        void Reset() override;

        void WaitIdle() const override;

        [[nodiscard]] std::shared_ptr<VulkanDevice> GetDevice() const;

        float GetAspectRatio() override;

    protected:
        virtual void AddResizeCallbacks();

    private:
        void InitVulkan();

        void CreateMissingTexture();

        virtual void CreateGraphicsResources();

        void CreatePipelinesAndShaders();

        void CreateBuffers();

        void CreateDescriptorAndSyncObjects();

        void RecreateSwapChain() const;

        void CreateSyncObjects();

        void CreateCommandBuffers();

        void CreateDescriptorSets();

        void CreateDescriptorPool();

        void CreateUniformBuffers();

        void CreateIndexBuffer();

        template<typename T>
        void UploadToBuffer(
            const VkBuffer &targetBuffer,
            const std::vector<T> &data,
            size_t size
        );

        void CreateVertexBuffer();

        void CopyBuffer(
            const VkBuffer &srcBuffer,
            const VkBuffer &dstBuffer,
            const VkDeviceSize &size
        ) const;

        void CreateTextureSampler();

        void CreateBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VmaMemoryUsage vmaUsage,
            VmaAllocationCreateFlags allocationFlags, VkBuffer &buffer, VmaAllocation &allocation, const char *debugName
        ) const;

        void CopyBufferToImage(
            const VkBuffer &buffer,
            const VkImage &image,
            uint32_t width,
            uint32_t height
        ) const;

        void CreateGraphicsPipeline();

        void CreateDescriptorSetLayout();

        void CreateMainRenderPass();

        void CreateSwapChain() const;

        void UpdateTextureDescriptor(
            TextureId textureId
        ) override;

        void UpdateGeometryBuffers() override;

        void DumpVmaStats() const;

        MemoryUsage GetMemoryUsage() override;
    };
}

#endif //GAME_ENGINE_VULKAN_RENDERER_H
