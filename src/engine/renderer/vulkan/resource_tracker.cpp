#include "resource_tracker.h"

#include "utils.h"

std::string LayoutToString(const VkImageLayout layout) {
    switch (layout) {
        case VK_IMAGE_LAYOUT_UNDEFINED: return "UNDEFINED";
        case VK_IMAGE_LAYOUT_GENERAL: return "GENERAL";
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return "COLOR_ATTACHMENT";
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return "DEPTH_STENCIL_ATTACHMENT";
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return "SHADER_READ_ONLY";
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return "TRANSFER_SRC";
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: return "TRANSFER_DST";
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: return "PRESENT_SRC";
        default: return "UNKNOWN(" + std::to_string(layout) + ")";
    }
}

void GetSyncSpecs(const VkImageLayout layout, VkAccessFlags &access, VkPipelineStageFlags &stage) {
    switch (layout) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            access = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            access = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            access = VK_ACCESS_SHADER_READ_BIT;
            stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            access = VK_ACCESS_TRANSFER_READ_BIT;
            stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            access = VK_ACCESS_TRANSFER_WRITE_BIT;
            stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            access = 0;
            stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            break;
        default:
            access = 0;
            stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            break;
    }
}

namespace Vulkan {
    void ResourceTracker::RegisterImage(
        const std::string &name,
        const VkImage &image,
        const VkFormat format,
        const VkImageLayout initialLayout
    ) {
        m_ImageStates[image] = {
            name,
            initialLayout,
            0,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            format
        };
    }

    void ResourceTracker::UnregisterImage(const VkImage &image) {
        const auto it = m_ImageStates.find(image);
        if (it != m_ImageStates.end()) {
            m_ImageStates.erase(it);
        }
    }

    void ResourceTracker::Transition(
        const VkCommandBuffer &cmd,
        const VkImage &image,
        const VkImageLayout newLayout
    ) {
        const auto &state = m_ImageStates[image];

        if (state.layout == newLayout) return;

        VkAccessFlags dstAccess;
        VkPipelineStageFlags dstStage;
        GetSyncSpecs(newLayout, dstAccess, dstStage);

        VkImageMemoryBarrier barrier{.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        barrier.oldLayout = state.layout;
        barrier.newLayout = newLayout;
        barrier.srcAccessMask = state.accessMask;
        barrier.dstAccessMask = dstAccess;
        barrier.image = image;
        barrier.subresourceRange = {0, 0, 1, 0, 1};

        if (
            newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            || Utils::IsDepthFormat(state.format)
        ) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (Utils::HasStencilComponent(state.format))
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        } else {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        vkCmdPipelineBarrier(
            cmd,
            state.stageMask,
            dstStage,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier
        );

        SetState(
            image,
            newLayout,
            dstAccess,
            dstStage
        );
    }

    void ResourceTracker::SetState(
        const VkImage &image,
        const VkImageLayout layout,
        const VkAccessFlags accessMask,
        const VkPipelineStageFlags stageMask
    ) {
        auto &state = m_ImageStates[image];
        state.layout = layout;
        state.accessMask = accessMask;
        state.stageMask = stageMask;
    }

    void ResourceTracker::DumpStates() const {
        std::cout << "--- ResourceTracker State Dump ---" << std::endl;
        if (m_ImageStates.empty()) {
            std::cout << "No tracked images." << std::endl;
            return;
        }

        for (const auto &[image, state]: m_ImageStates) {
            std::cout << "Image :       " << state.name << std::endl
                    << " | Handle:      " << image << std::endl
                    << " | Layout:      " << LayoutToString(state.layout) << std::endl
                    << " | Access Mask: " << state.accessMask << std::endl
                    << " | Stage Mask:  " << state.stageMask << std::endl
                    << " | Format:      " << state.format << std::endl
                    << std::endl;
        }
        std::cout << "----------------------------------" << std::endl;
    }
}
