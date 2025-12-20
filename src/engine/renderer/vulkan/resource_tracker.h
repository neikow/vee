#ifndef VEE_RESOURCE_TRACKER_H
#define VEE_RESOURCE_TRACKER_H
#include <string>
#include <unordered_map>
#include <vulkan/vulkan_core.h>

namespace Vulkan {
    /** Structure to hold the state of a Vulkan resource (image).
     * It includes layout, access mask, stage mask, and format.
     */
    struct ResourceState {
        std::string name;
        VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkAccessFlags accessMask = 0;
        VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkFormat format = VK_FORMAT_UNDEFINED;
    };

    /** Class to track and manage the states of Vulkan resources (images).
     * It allows registering images and transitioning their states.
     */
    class ResourceTracker {
        std::unordered_map<VkImage, ResourceState> m_ImageStates;

    public:
        /** Register a Vulkan image with its initial format and layout.
         * @param image The Vulkan image to register.
         * @param format The format of the image.
         * @param initialLayout The initial layout of the image.
         */
        void RegisterImage(
            const std::string &name,
            const VkImage &image,
            VkFormat format,
            VkImageLayout initialLayout
        );

        /** Unregister a Vulkan image from tracking.
         * @param image The Vulkan image to unregister.
         */
        void UnregisterImage(
            const VkImage &image
        );

        /** Transition a Vulkan image to a new layout, access mask, and stage mask.
         * @param cmd The command buffer to record the transition commands.
         * @param image The Vulkan image to transition.
         * @param newLayout The new layout for the image.
         */
        void Transition(
            const VkCommandBuffer &cmd,
            const VkImage &image,
            VkImageLayout newLayout
        );

        void SetState(
            const VkImage &image,
            VkImageLayout layout,
            VkAccessFlags accessMask,
            VkPipelineStageFlags stageMask
        );

        void DumpStates() const;
    };
}

#endif
