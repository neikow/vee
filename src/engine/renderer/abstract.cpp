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

void AbstractRenderer::EnqueueCleanupTask(const RendererCleanupTask &task) {
        m_CleanupStack.push_back(task);
}

void AbstractRenderer::ExecuteInitTasks() {
        for (const auto &task: m_InitQueue) {
                task();
        }
        m_InitQueue.clear();
}

void AbstractRenderer::ExecuteCleanupTasks() {
        for (int i = m_CleanupStack.size() - 1; i >= 0; i--) {
                m_CleanupStack[i]();
        }
        m_CleanupStack.clear();
}


