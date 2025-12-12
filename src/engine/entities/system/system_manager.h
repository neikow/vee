#ifndef GAME_ENGINE_SYSTEM_MANAGER_H
#define GAME_ENGINE_SYSTEM_MANAGER_H
#include <map>
#include <ranges>
#include <typeindex>

#include "../manager.h"
#include "system.h"

class ComponentManager;

class SystemManager {
    std::map<std::type_index, Signature> m_Signatures;
    std::map<std::type_index, std::shared_ptr<SystemBase> > m_Systems;

public:
    template<typename T>
    std::shared_ptr<T> RegisterSystem(const std::shared_ptr<T> &system) {
        const std::type_index typeId = typeid(T);
        m_Systems[typeId] = system;
        return system;
    }

    template<typename T>
    void SetSignature(const Signature signature) {
        const std::type_index typeId = typeid(T);
        m_Signatures[typeId] = signature;
    }

    void EntitySignatureChanged(
        const EntityID entity,
        const Signature entitySignature
    ) {
        for (const auto &pair: m_Systems) {
            const std::type_index &typeId = pair.first;
            auto &system = pair.second;

            const Signature &systemSignature = m_Signatures[typeId];

            // Check if the entity's signature *matches* the system's required signature
            if ((entitySignature & systemSignature) == systemSignature) {
                // Entity is now relevant: Add to the system
                system->m_Entities.insert(entity);
            } else {
                // Entity is no longer relevant: Remove from the system
                system->m_Entities.erase(entity);
            }
        }
    }

    void UpdateSystems(const float deltaTime) {
        for (const auto &val: m_Systems | std::views::values) {
            auto &system = val;
            system->Update(deltaTime);
        }
    }

    void RemoveEntity(const EntityID entity) {
        return EntitySignatureChanged(entity, 0);
    };
};


#endif //GAME_ENGINE_SYSTEM_MANAGER_H
