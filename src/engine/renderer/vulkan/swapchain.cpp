#include "swapchain.h"
#include "vulkan_device.h"
#include "utils.h"

namespace Vulkan {
    Swapchain::Swapchain(
        const std::shared_ptr<VulkanDevice> &device,
        const std::shared_ptr<ResourceTracker> &tracker
    )
        : m_Device(device),
          m_ResourceTracker(tracker) {
    }

    void Swapchain::Create(
        const VkExtent2D &extent,
        const VkRenderPass &renderPass
    ) {
        CreateDepthResources(extent);

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

        if (
            vkCreateSwapchainKHR(
                m_Device->GetLogicalDevice(),
                &createInfo,
                nullptr,
                &m_Swapchain
            ) != VK_SUCCESS
        ) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(
            m_Device->GetLogicalDevice(),
            m_Swapchain,
            &imageCount,
            nullptr
        );
        m_Images.resize(imageCount);
        m_ImageViews.resize(imageCount);
        m_Framebuffers.resize(imageCount);
        vkGetSwapchainImagesKHR(
            m_Device->GetLogicalDevice(),
            m_Swapchain,
            &imageCount,
            m_Images.data()
        );

        for (size_t i = 0; i < imageCount; i++) {
            m_ResourceTracker->RegisterImage(
                "Swapchain_Image_" + std::to_string(i),
                m_Images[i],
                m_Format,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
            );

            m_ImageViews[i] = m_Device->CreateImageView(
                m_Images[i],
                m_Format,
                VK_IMAGE_ASPECT_COLOR_BIT
            );

            std::array attachments = {m_ImageViews[i], m_DepthImageView};

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

        for (const auto image: m_Images) {
            m_ResourceTracker->UnregisterImage(image);
        }

        m_Device->DestroySwapchain(m_Swapchain);

        m_Device->DestroyImageView(m_DepthImageView);
        m_Device->DestroyImage(m_DepthImage, m_DepthImageAllocation);
    }

    void Swapchain::CreateDepthResources(const VkExtent2D &extent) {
        const VkFormat depthFormat = Utils::FindDepthFormat(m_Device->GetPhysicalDevice());

        Utils::CreateImage(
            m_Device,
            extent.width,
            extent.height,
            depthFormat,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            m_DepthImage,
            m_DepthImageAllocation,
            "Depth Image"
        );

        m_ResourceTracker->RegisterImage(
            "Swapchain_Depth_Image",
            m_DepthImage,
            depthFormat,
            VK_IMAGE_LAYOUT_UNDEFINED
        );

        m_DepthImageView = m_Device->CreateImageView(
            m_DepthImage,
            depthFormat,
            VK_IMAGE_ASPECT_DEPTH_BIT
        );
    }
}
