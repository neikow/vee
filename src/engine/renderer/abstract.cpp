#include "abstract.h"

AbstractRenderer::AbstractRenderer() : m_MeshManager(
                                               std::make_shared<MeshManager>(this)
                                       ),
                                       m_TextureManager(
                                               std::make_shared<Vulkan::TextureManager>(this)
                                       ) {
}

void AbstractRenderer::EnqueuePostInitTask(const RendererInitTask &task) {
        if (m_Initialized) {
                task();
        } else {
                m_InitQueue.push_back(task);
        }
}

void AbstractRenderer::ExecuteInitTasks() {
        for (const auto &task: m_InitQueue) {
                task();
        }
        m_InitQueue.clear();
}


