#include "swapchain.h"
#include "vulkan_device.h"
#include "utils.h"

namespace Vulkan {
    Swapchain::Swapchain(const std::shared_ptr<VulkanDevice> &device)
        : m_Device(device) {
    }

    void Swapchain::Create(
        const VkExtent2D &extent,
        const VkRenderPass &renderPass,
        const VkImageView &depthView
    ) {
        const auto swapChainSupport = m_Device->QuerySwapChainSupport(m_Device->GetPhysicalDevice());

        const VkSurfaceFormatKHR surfaceFormat = Utils::ChooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = Utils::ChooseSwapPresentMode(swapChainSupport.presentModes);
        m_Extent = Utils::ChooseSwapExtent(extent, swapChainSupport.capabilities);
        m_Format = surfaceFormat.format;

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (
            swapChainSupport.capabilities.maxImageCount > 0
            && imageCount > swapChainSupport.capabilities.maxImageCount
        ) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_Device->GetSurface();
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = m_Format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = m_Extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        const QueueFamilyIndices indices = m_Device->FindQueueFamilies(m_Device->GetPhysicalDevice());
        const uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(m_Device->GetLogicalDevice(), &createInfo, nullptr, &m_Swapchain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(m_Device->GetLogicalDevice(), m_Swapchain, &imageCount, nullptr);
        m_Images.resize(imageCount);
        vkGetSwapchainImagesKHR(m_Device->GetLogicalDevice(), m_Swapchain, &imageCount, m_Images.data());

        for (auto image: m_Images) {
            m_ImageViews.push_back(m_Device->CreateImageView(image, m_Format, VK_IMAGE_ASPECT_COLOR_BIT));
        }

        m_Framebuffers.resize(m_ImageViews.size());
        for (size_t i = 0; i < m_ImageViews.size(); i++) {
            std::array attachments = {m_ImageViews[i], depthView};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = attachments.size();
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = m_Extent.width;
            framebufferInfo.height = m_Extent.height;
            framebufferInfo.layers = 1;

            m_Framebuffers[i] = m_Device->CreateFramebuffer(
                framebufferInfo
            );
        }
    }

    void Swapchain::Cleanup() const {
        for (const auto framebuffer: m_Framebuffers) {
            m_Device->DestroyFramebuffer(framebuffer);
        }

        for (const auto imageView: m_ImageViews) {
            m_Device->DestroyImageView(imageView);
        }

        m_Device->DestroySwapchain(m_Swapchain);
    }
}
