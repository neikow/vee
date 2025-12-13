#ifndef VEE_CHILDREN_COMPONENT_H
#define VEE_CHILDREN_COMPONENT_H
#include <set>
#include "../../types.h"

struct ChildrenComponent {
    std::set<Entities::EntityID> children;
};

#endif //VEE_CHILDREN_COMPONENT_H
