#ifndef VEE_VULKAN_SWAPCHAIN_H
#define VEE_VULKAN_SWAPCHAIN_H
#include <memory>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace Vulkan {
    class ResourceTracker;
}

namespace Vulkan {
    class VulkanDevice;

    /** Swapchain class handles the creation and management of the Vulkan swapchain. */
    class Swapchain {
        std::shared_ptr<VulkanDevice> m_Device;
        std::shared_ptr<ResourceTracker> m_ResourceTracker;

        VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
        VkExtent2D m_Extent{};
        VkFormat m_Format = VK_FORMAT_UNDEFINED;

        std::vector<VkImage> m_Images;
        std::vector<VkImageView> m_ImageViews;
        std::vector<VkFramebuffer> m_Framebuffers;

        VkImage m_DepthImage = VK_NULL_HANDLE;
        VmaAllocation m_DepthImageAllocation = VK_NULL_HANDLE;
        VkImageView m_DepthImageView = VK_NULL_HANDLE;

    public:
        /** Constructor that initializes the Swapchain with a VulkanDevice. */
        explicit Swapchain(
            const std::shared_ptr<VulkanDevice> &device,
            const std::shared_ptr<ResourceTracker> &tracker
        );

        ~Swapchain() = default;

        /** Creates the Vulkan swapchain. */
        void Create(
            const VkExtent2D &extent,
            const VkRenderPass &renderPass
        );

        /** Cleans up the Vulkan swapchain resources. */
        void Cleanup() const;

        void CreateDepthResources(const VkExtent2D &extent);

        [[nodiscard]] VkSwapchainKHR GetHandle() const { return m_Swapchain; }
        [[nodiscard]] VkExtent2D GetExtent() const { return m_Extent; }
        [[nodiscard]] VkFormat GetFormat() const { return m_Format; }
        [[nodiscard]] VkFramebuffer GetFramebuffer(const uint32_t index) const { return m_Framebuffers[index]; }
        [[nodiscard]] VkImage GetImage(const size_t index) const { return m_Images[index]; }
        [[nodiscard]] VkImageView GetDepthImageView() const { return m_DepthImageView; }
        [[nodiscard]] size_t GetImageCount() const { return m_Images.size(); }
    };
}

#endif
