#include "deletion_queue.h"

DeletionQueue::DeletionQueue(const std::uint32_t framesInFlight) : m_FramesInFlight(framesInFlight) {
}

void DeletionQueue::Enqueue(const Deletion &deletion) {
    m_DeletionQueue.push(deletion);
}

void DeletionQueue::Flush(const std::uint32_t currentFrame) {
    while (
        !m_DeletionQueue.empty() &&
        m_DeletionQueue.front().deletionFrame + m_FramesInFlight < currentFrame
    ) {
        m_DeletionQueue.front().cleanupFunc();
        m_DeletionQueue.pop();
    }
}
