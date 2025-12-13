#ifndef VEE_TRANSFORM_SYSTEM_H
#define VEE_TRANSFORM_SYSTEM_H
#include "system.h"
#include "../../utils/math_utils.h"
#include "../../utils/macros/log_macros.h"

#include "../components_system/component_manager.h"
#include "../components_system/components/local_to_world_component.h"
#include "../components_system/components/local_transform_component.h"
#include "../components_system/components/parent_component.h"


class TransformSystem final : public SystemBase {
public:
    using SystemBase::SystemBase;

    /** Recursively computes the world transform matrix for the given entity,
     * taking into account its local transform and the transforms of its parent entities.
     *
     * TODO: This function currently does not take advantage of the cached matrices.
     *
     * @param entity The ID of the entity for which to compute the world transform matrix.
     * @return The computed world transform matrix as a `glm::mat4`.
     */
    glm::mat4 ComputeAndCacheTransformMatrix(const EntityID entity) {
        LocalTransformComponent localTransform;
        // Get the local transform; if it doesn't exist, use default values to handle simple groups definitions
        // (without local transform).
        if (m_ComponentManager->HasComponent<LocalTransformComponent>(entity)) {
            localTransform = m_ComponentManager->GetComponent<LocalTransformComponent>(entity);
        } else {
            localTransform.position = glm::vec3(0.0f, 0.0f, 0.0f);
            localTransform.rotation = glm::quat(1, 0, 0, 0);
            localTransform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
        }
        auto &localToWorld = m_ComponentManager->GetComponent<LocalToWorldComponent>(entity);

        // If the local to world matrix is not dirty, return the cached matrix.
        if (!localToWorld.isDirty) {
            return localToWorld.localToWorldMatrix;
        }

        glm::mat4 localToWorldMatrix = Utils::Math::CalculateWorldMatrix(
            localTransform.position, localTransform.rotation, localTransform.scale
        );

        if (m_ComponentManager->HasComponent<ParentComponent>(entity)) {
            const auto &parentData = m_ComponentManager->GetComponent<ParentComponent>(entity);
            if (parentData.parent != NULL_ENTITY) {
                // Recursively compute the parent's world matrix and combine it with the local matrix.
                localToWorldMatrix = ComputeAndCacheTransformMatrix(parentData.parent) * localToWorldMatrix;
            } else {
                LOG_WARN("Entity " + std::to_string(entity) + " parent is NULL_ENTITY");
            }
        }

        // Cache the computed matrix and mark it as not dirty.
        localToWorld.localToWorldMatrix = localToWorldMatrix;
        // localToWorld.isDirty = false;

        return localToWorldMatrix;
    }

    void Update(const float dt) override {
        for (const auto entity: m_Entities) {
            // Simply compute and cache the transform matrix for each entity.
            ComputeAndCacheTransformMatrix(entity);
        }
    }
};

#endif //VEE_TRANSFORM_SYSTEM_H
