#include "hierarchy.h"

#include <memory>

#include "../../entities/components_system/component_manager.h"
#include "../../entities/components_system/components/children_component.h"
#include "../../entities/components_system/components/parent_component.h"

namespace Utils::Entities::Hierarchy {
    void SetParent(
        const EntityID child,
        const EntityID newParent,
        const std::shared_ptr<ComponentManager> &componentManager
    ) {
        if (componentManager->HasComponent<ParentComponent>(child)) {
            const auto &oldParentComp = componentManager->GetComponent<ParentComponent>(child);
            const EntityID oldParent = oldParentComp.parent;
            if (oldParent != NULL_ENTITY && componentManager->HasComponent<ChildrenComponent>(oldParent)) {
                auto &oldParentChildrenComp = componentManager->GetComponent<ChildrenComponent>(oldParent);
                oldParentChildrenComp.children.erase(child);
            }
        }
        if (!componentManager->HasComponent<ParentComponent>(child)) {
            componentManager->AddComponent<ParentComponent>(
                child,
                ParentComponent{NULL_ENTITY}
            );
        }
        auto &childParentComp = componentManager->GetComponent<ParentComponent>(child);
        childParentComp.parent = newParent;
        if (newParent != NULL_ENTITY) {
            if (!componentManager->HasComponent<ChildrenComponent>(newParent)) {
                componentManager->AddComponent<ChildrenComponent>(
                    newParent,
                    ChildrenComponent{}
                );
            }
            auto &newParentChildrenComp = componentManager->GetComponent<ChildrenComponent>(newParent);
            newParentChildrenComp.children.insert(child);
        }
    }

    void AddChild(
        const EntityID parent,
        const EntityID child,
        const std::shared_ptr<ComponentManager> &componentManager
    ) {
        if (!componentManager->HasComponent<ParentComponent>(child)) {
            componentManager->AddComponent<ParentComponent>(
                child,
                ParentComponent{NULL_ENTITY}
            );
        }
        auto &childParentComp = componentManager->GetComponent<ParentComponent>(child);
        childParentComp.parent = parent;
        if (!componentManager->HasComponent<ChildrenComponent>(parent)) {
            componentManager->AddComponent<ChildrenComponent>(
                parent,
                ChildrenComponent{}
            );
        }
        auto &parentChildrenComp = componentManager->GetComponent<ChildrenComponent>(parent);
        parentChildrenComp.children.insert(child);
    }
}
