#ifndef VEE_DELETION_QUEUE_H
#define VEE_DELETION_QUEUE_H

#include <functional>
#include <queue>

/** Represents a cleanup task to be executed after a certain number of frames have passed.
 */
struct Deletion {
    std::function<void()> cleanupFunc;
    std::uint32_t deletionFrame;
};

/** A queue that manages deferred deletion of resources.
 *
 * Resources are enqueued for deletion and are only deleted after a specified number of frames have passed.
 */
class DeletionQueue {
    std::queue<Deletion> m_DeletionQueue;
    std::uint32_t m_FramesInFlight;

public:
    /** Constructs a DeletionQueue.
     *
     * @param framesInFlight The number of frames to wait before executing deletion tasks.
     */
    explicit DeletionQueue(
        std::uint32_t framesInFlight
    );

    /** Enqueues a deletion task.
     *
     * @param deletion The deletion task to enqueue.
     */
    void Enqueue(const Deletion &deletion);

    /** Flushes the deletion queue, executing tasks that are due for deletion.
     *
     * @param currentFrame The current frame number.
     */
    void Flush(std::uint32_t currentFrame);
};


#endif //VEE_DELETION_QUEUE_H
