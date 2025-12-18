#ifndef GAME_ENGINE_VULKAN_RENDERER_H
#define GAME_ENGINE_VULKAN_RENDERER_H

#define GLFW_INCLUDE_VULKAN
#include <vk_mem_alloc.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#include "shader_module_cache.h"
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
        std::shared_ptr<ShaderModuleCache> m_ShaderModuleCache;

        VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
        std::vector<VkImage> m_SwapChainImages;
        VkFormat m_SwapChainImageFormat = VK_FORMAT_UNDEFINED;
        VkExtent2D m_SwapChainExtent = {};
        std::vector<VkImageView> m_SwapChainImageViews;
        std::vector<VkFramebuffer> m_SwapChainFramebuffers;

        VkRenderPass m_MainRenderPass = VK_NULL_HANDLE;

        VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> m_DescriptorSets;

        VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_GraphicsPipeline = VK_NULL_HANDLE;

        std::vector<VkCommandBuffer> m_CommandBuffers;

        VkImage m_DepthImage = VK_NULL_HANDLE;
        VmaAllocation m_DepthImageAllocation = VK_NULL_HANDLE;
        VkImageView m_DepthImageView = VK_NULL_HANDLE;

        VkSampler m_TextureSampler = VK_NULL_HANDLE;

        std::vector<DrawCall> m_DrawQueue;

        VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
        VmaAllocation m_VertexAllocation = VK_NULL_HANDLE;
        VkBuffer m_IndexBuffer = VK_NULL_HANDLE;
        VmaAllocation m_IndexAllocation = VK_NULL_HANDLE;

        std::vector<VkBuffer> m_UniformBuffers;
        std::vector<VmaAllocation> m_UniformBuffersAllocations;
        std::vector<void *> m_UniformBuffersMapped;

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

    public:
        explicit Renderer(const std::shared_ptr<Window> &window);

        void Initialize(const std::string &appName, uint32_t version) override;

        void RecordDrawQueue(const VkCommandBuffer &commandBuffer);

        void RenderScene(const VkCommandBuffer &commandBuffer, const VkFramebuffer &framebuffer,
                         VkExtent2D extent) const;

        virtual void RenderToScreen(const VkCommandBuffer &cmd);

        void Draw() override;

        void UpdateCameraMatrix(const glm::mat4x4 &viewMatrix, const glm::mat4x4 &projectionMatrix) override;

        void SubmitDrawCall(std::uint32_t entityId, const glm::mat4x4 &worldMatrix, uint32_t meshId,
                            uint32_t textureId) override;

        void Cleanup() override;

        void Reset() override;

        void WaitIdle() const override;

        [[nodiscard]] std::shared_ptr<VulkanDevice> GetDevice() const;

        float GetAspectRatio() override;

    private:
        void InitVulkan();

        void CreateMissingTexture();

        virtual void CreateGraphicsResources();

        void SetSwapChainImageFormat();

        void CreatePipelinesAndShaders();

        void CreateBuffers();

        void CreateDescriptorAndSyncObjects();

        void RecreateSwapChain();

        void CleanupSwapChain() const;

        void CreateSyncObjects();

        void CreateCommandBuffers();

        void CreateDescriptorSets();

        void CreateDescriptorPool();

        void CreateUniformBuffers();

        void CreateIndexBuffer();

        template<typename T>
        void UploadToBuffer(const VkBuffer &targetBuffer, const std::vector<T> &data, size_t size);

        void CreateVertexBuffer();

        void CopyBuffer(
            VkBuffer srcBuffer,
            VkBuffer dstBuffer,
            VkDeviceSize size
        );

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

        void CreateFramebuffers();

        void CreateImage(
            uint32_t width,
            uint32_t height,
            VkFormat format,
            VkImageTiling tiling,
            VkImageUsageFlags usage,
            VmaMemoryUsage vmaUsage, VkImage &image, VmaAllocation &allocation, const char *debugName
        ) const;

        [[nodiscard]] VkCommandBuffer BeginSingleTimeCommands(const VkCommandPool &commandPool) const;

        void EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool commandPool) const;

        void TransitionImageLayout(
            const VkImage &image,
            VkFormat format,
            VkImageLayout oldLayout,
            VkImageLayout newLayout
        ) const;

        void CreateDepthResources();

        [[nodiscard]] VkShaderModule CreateShaderModule(const std::vector<uint32_t> &code) const;

        void CreateGraphicsPipeline();

        void CreateDescriptorSetLayout();

        void CreateMainRenderPass();

        [[nodiscard]] VkImageView CreateImageView(
            const VkImage &image,
            VkFormat format,
            VkImageAspectFlags aspectFlags
        ) const;

        void CreateImageViews();

        void CreateSwapChain();

        static void FramebufferResizeCallback(GLFWwindow *window, int, int);

        void UpdateTextureDescriptor(TextureId textureId) override;

        void UpdateGeometryBuffers() override;

        void DumpVmaStats() const;

        MemoryUsage GetMemoryUsage() override;
    };
}

#endif //GAME_ENGINE_VULKAN_RENDERER_H
