#ifndef VEE_LOCAL_TO_WORLD_COMPONENT_H
#define VEE_LOCAL_TO_WORLD_COMPONENT_H
#include <glm/glm.hpp>

struct LocalToWorldComponent final {
    /** Cached local to world transformation matrix.
     */
    glm::mat4 localToWorldMatrix{};
    /** Flag indicating whether the local to world matrix needs to be recomputed.
     *
     * TODO: Currently always set to true. Implement proper dirty flag logic.
     */
    bool isDirty = true;
};

#endif //VEE_LOCAL_TO_WORLD_COMPONENT_H
