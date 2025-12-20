#ifndef VEE_RENDER_GRAPH_H
#define VEE_RENDER_GRAPH_H

#include <vulkan/vulkan.h>
#include <string>
#include <functional>

namespace Vulkan {
    class ResourceTracker;

    /** Structure representing the usage of a resource within a render pass.
     * It includes the image, its layout, access flags, and pipeline stage flags.
     */
    struct ResourceUsage {
        /** The Vulkan image resource being used. */
        VkImage image;
        /** The layout of the image during the render pass. */
        VkImageLayout layout;
        /** Access flags indicating how the image is accessed. */
        VkAccessFlags access;
        /** Pipeline stage flags indicating the pipeline stages involved. */
        VkPipelineStageFlags stage;
    };

    /** A node representing a render pass in the render graph.
     * It contains the name, execution function, and resource usages.
     */
    struct RenderPassNode {
        /** Name of the render pass for identification. */
        std::string name;
        /** Function to execute the render pass, taking a command buffer as input. */
        std::function<void(VkCommandBuffer)> execute;
        /** List of resource usages for this render pass. */
        std::vector<ResourceUsage> usages;
    };

    /** A simple render graph that manages render passes and resource transitions.
     * It allows adding render passes with their resource usages and executes them
     * in order while handling necessary resource transitions.
     */
    class RenderGraph {
        std::vector<RenderPassNode> m_Nodes;
        std::shared_ptr<ResourceTracker> m_Tracker;

    public:
        /** Construct a RenderGraph with a given ResourceTracker.
         * @param tracker Shared pointer to the ResourceTracker for managing resource states.
         */
        explicit RenderGraph(
            const std::shared_ptr<ResourceTracker> &tracker
        );

        /** Add a render pass node to the graph.
         * @param node The RenderPassNode containing execution logic and resource usages.
         */
        void AddPass(RenderPassNode &&node);

        /** Execute all render passes in the graph, handling resource transitions.
         * @param cmd The command buffer to record commands into.
         */
        void Execute(const VkCommandBuffer &cmd);
    };
}

#endif
