#include "render_graph.h"

#include <iostream>

#include "resource_tracker.h"

namespace Vulkan {
    RenderGraph::RenderGraph(const std::shared_ptr<ResourceTracker> &tracker) : m_Tracker(tracker) {
    }

    void RenderGraph::AddPass(RenderPassNode &&node) {
        m_Nodes.push_back(std::move(node));
    }

    void RenderGraph::Execute(const VkCommandBuffer &cmd) {
        for (const auto &node: m_Nodes) {
            // TODO: check if transition/render cmd split is needed for optimization

            // std::cout << "Executing Render Pass: " << node.name << std::endl;
            // m_Tracker->DumpStates();
            // std::cout << std::endl << std::endl;

            for (const auto &usage: node.usages) {
                m_Tracker->Transition(
                    cmd,
                    usage.image,
                    usage.layout
                );
            }

            node.execute(cmd);
        }


        m_Nodes.erase(m_Nodes.begin(), m_Nodes.end());
    }
} // Vulkan
