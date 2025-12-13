#ifndef VEE_UTILS_ENTITIES_HIERARCHY_H
#define VEE_UTILS_ENTITIES_HIERARCHY_H
#include <memory>

#include "../../entities/types.h"

class ComponentManager;
using namespace Entities;

namespace Utils::Entities::Hierarchy {
    /** Sets a new parent for the specified child entity.
     *
     * This function updates the ParentComponent of the child entity to reference the new parent entity.
     * It also updates the ChildrenComponent of both the old and new parent entities to reflect the change
     * in hierarchy.
     *
     * @param child The ID of the child entity whose parent is to be changed.
     * @param newParent The ID of the new parent entity.
     * @param componentManager A shared pointer to the ComponentManager for accessing and modifying components.
     * @param entityManager A shared pointer to the EntityManager for managing entities.
     */
    void SetParent(
        EntityID child,
        EntityID newParent,
        const std::shared_ptr<ComponentManager> &componentManager
    );

    /** Adds a child entity to the specified parent entity.
     *
     * This function updates the ParentComponent of the child entity to reference the parent entity.
     * It also updates the ChildrenComponent of the parent entity to include the new child.
     *
     * @param parent The ID of the parent entity.
     * @param child The ID of the child entity to be added.
     * @param componentManager A shared pointer to the ComponentManager for accessing and modifying components.
     * @param entityManager A shared pointer to the EntityManager for managing entities.
     */
    void AddChild(
        EntityID parent,
        EntityID child,
        const std::shared_ptr<ComponentManager> &componentManager
    );
}

#endif


