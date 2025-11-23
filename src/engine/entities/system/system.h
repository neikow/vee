#ifndef GAME_ENGINE_BASE_H
#define GAME_ENGINE_BASE_H
#include <set>

#include "../manager.h"

class ComponentManager;

class SystemBase {
protected:
    std::shared_ptr<ComponentManager> m_ComponentManager;

public:
    explicit SystemBase(
        const std::shared_ptr<ComponentManager> &componentManager
    )
        : m_ComponentManager(componentManager) {
    }

    std::set<EntityID> m_Entities;

    virtual void Update(float dt) = 0;

    virtual ~SystemBase() = default;
};


#endif //GAME_ENGINE_BASE_H
