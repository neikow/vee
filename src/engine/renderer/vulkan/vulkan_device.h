#ifndef VEE_VULKAN_DEVICE_H
#define VEE_VULKAN_DEVICE_H

#include <map>
#include <set>

#include "types.h"
#include "vulkan_renderer.h"

constexpr auto SEVERITY =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

constexpr auto MESSAGE_TYPE =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

class Window;

namespace Vulkan {
    class VulkanDevice {
        std::shared_ptr<Window> m_Window;

        VkInstance m_Instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
        VkDevice m_Device = VK_NULL_HANDLE;

        VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
        VkQueue m_PresentQueue = VK_NULL_HANDLE;
        VkQueue m_TransferQueue = VK_NULL_HANDLE;

        VkCommandPool m_GraphicsCommandPool = VK_NULL_HANDLE;
        VkCommandPool m_TransferCommandPool = VK_NULL_HANDLE;

        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

        VmaAllocator m_Allocator = VK_NULL_HANDLE;

    public:
        explicit VulkanDevice(const std::shared_ptr<Window> &window);

        [[nodiscard]] VkDevice &GetLogicalDevice();

        [[nodiscard]] VkPhysicalDevice &GetPhysicalDevice();

        [[nodiscard]] VkInstance &GetInstance();

        [[nodiscard]] VkQueue &GetGraphicsQueue();

        [[nodiscard]] VkQueue &GetPresentQueue();

        [[nodiscard]] VkQueue &GetTransferQueue();

        [[nodiscard]] VmaAllocator &GetAllocator();

        [[nodiscard]] VkSurfaceKHR &GetSurface();

        void Initialize(const std::string &appName, const uint32_t version);

        void WaitIdle() const;

        void CreateSurface();

        void CreateLogicalDevice();

        void PickPhysicalDevice();

        void SetupDebugMessenger();

        void CreateInstance(
            const std::string &appName,
            uint32_t version
        );

        int RateDeviceSuitability(const VkPhysicalDevice &physicalDevice) const;

        QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice &physicalDevice) const;

        void CreateCommandPools();

        SwapChainSupportDetails QuerySwapChainSupport(const VkPhysicalDevice &device) const;

        void Cleanup() const;

        void DestroyImageView(const VkImageView &imageView) const;

        void DestroyImage(const VkImage &image, const VmaAllocation &allocation) const;

        void MapMemory(const VmaAllocation &allocation, void **data) const;

        void UnmapMemory(const VmaAllocation &allocation) const;

        void DestroyBuffer(const VkBuffer &buffer, const VmaAllocation &allocation) const;

        VkCommandPool GetTransferCommandPool() const;

        VkCommandPool GetGraphicsCommandPool() const;

        VmaAllocationInfo GetAllocationInfo(const VmaAllocation &allocation) const;

        VkFramebuffer CreateFramebuffer(const VkFramebufferCreateInfo &framebufferCreateInfo) const;

        VkRenderPass CreateRenderPass(const VkRenderPassCreateInfo &renderPassCreateInfo) const;

        VkPipeline CreatePipeline(const VkGraphicsPipelineCreateInfo &pipelineCreateInfo) const;

        void DestroyFramebuffer(const VkFramebuffer &framebuffer) const;

        void DestroyShaderModule(const VkShaderModule &shaderModule) const;

        VkDescriptorPool CreateDescriptorPool(const VkDescriptorPoolCreateInfo &descriptorPoolCreateInfo) const;

        void DestroyDescriptorPool(const VkDescriptorPool &descriptorPool) const;

        void DestroyRenderPass(const VkRenderPass &renderPass) const;

        void DestroyPipeline(const VkPipeline &pipeline) const;

        void DestroySemaphore(const VkSemaphore &semaphore) const;

        void DestroyFence(const VkFence &fence) const;

        void DestroyPipelineLayout(const VkPipelineLayout &pipelineLayout) const;

        void DestroyDescriptorSetLayout(const VkDescriptorSetLayout &descriptorSetLayout) const;

        void DestroySampler(const VkSampler &sampler) const;

        void DestroySwapchain(const VkSwapchainKHR &swapchain) const;
    };
}


#endif //VEE_VULKAN_DEVICE_H
