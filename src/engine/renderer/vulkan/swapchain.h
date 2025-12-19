#ifndef VEE_VULKAN_SWAPCHAIN_H
#define VEE_VULKAN_SWAPCHAIN_H
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace Vulkan {
    class VulkanDevice;

    /** Swapchain class handles the creation and management of the Vulkan swapchain. */
    class Swapchain {
        std::shared_ptr<VulkanDevice> m_Device;

        VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
        VkExtent2D m_Extent{};
        VkFormat m_Format = VK_FORMAT_UNDEFINED;

        std::vector<VkImage> m_Images;
        std::vector<VkImageView> m_ImageViews;
        std::vector<VkFramebuffer> m_Framebuffers;

    public:
        /** Constructor that initializes the Swapchain with a VulkanDevice. */
        explicit Swapchain(const std::shared_ptr<VulkanDevice> &device);

        ~Swapchain() = default;

        /** Creates the Vulkan swapchain. */
        void Create(const VkExtent2D &extent, const VkRenderPass &renderPass, const VkImageView &depthView);

        /** Cleans up the Vulkan swapchain resources. */
        void Cleanup() const;

        [[nodiscard]] VkSwapchainKHR GetHandle() const { return m_Swapchain; }
        [[nodiscard]] VkExtent2D GetExtent() const { return m_Extent; }
        [[nodiscard]] VkFormat GetFormat() const { return m_Format; }
        [[nodiscard]] VkFramebuffer GetFramebuffer(const uint32_t index) const { return m_Framebuffers[index]; }
        [[nodiscard]] size_t GetImageCount() const { return m_Images.size(); }
    };
}

#endif
