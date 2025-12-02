#ifndef GAME_ENGINE_COMPONENT_MANAGER_H
#define GAME_ENGINE_COMPONENT_MANAGER_H
#include <unordered_map>

#include "component_array.h"
#include "../manager.h"
#include "../system/system_manager.h"

class EntityManager;

class ComponentManager {
    std::shared_ptr<EntityManager> m_EntityManager;
    std::shared_ptr<SystemManager> m_SystemManager;
    std::array<std::shared_ptr<IComponentArray>, MAX_COMPONENTS> m_ComponentArrays;
    std::unordered_map<const char *, ComponentTypeId> m_ComponentTypeMap;

    template<typename T>
    std::shared_ptr<ComponentArray<T> > GetComponentArray() {
        const ComponentTypeId typeID = ComponentTypeHelper<T>::ID;
        return std::static_pointer_cast<ComponentArray<T> >(m_ComponentArrays[typeID]);
    }

public:
    ComponentManager(
        const std::shared_ptr<SystemManager> &systemManager,
        const std::shared_ptr<EntityManager> &entityManager
    ) : m_EntityManager(entityManager), m_SystemManager(systemManager) {
    }

    template<typename T>
    void RegisterComponent() {
        const char *typeName = typeid(T).name();
        ComponentTypeId typeID = ComponentTypeHelper<T>::ID;
        m_ComponentTypeMap.insert({typeName, typeID});
        m_ComponentArrays[typeID] = std::make_shared<ComponentArray<T> >();
    }

    template<typename T>
    T AddComponent(EntityID entity, T component) {
        GetComponentArray<T>()->InsertData(entity, component);

        Signature signature = m_EntityManager->GetSignature(entity);
        const ComponentTypeId typeID = ComponentTypeHelper<T>::ID;

        signature.set(typeID);

        m_EntityManager->SetSignature(entity, signature);
        m_SystemManager->EntitySignatureChanged(entity, signature);

        return component;
    }

    template<typename T>
    T &GetComponent(EntityID entity) {
        return GetComponentArray<T>()->GetData(entity);
    }

    void EntityDestroyed(const EntityID entity) const {
        for (auto const &arr: m_ComponentArrays) {
            if (arr) {
                arr->RemoveEntity(entity);
            }
        }
    }

    [[nodiscard]] std::vector<ComponentTypeId> GetEntityComponents(const EntityID entity) const {
        std::vector<ComponentTypeId> components;
        for (auto const &arr: m_ComponentArrays) {
            if (arr && arr->HasData(entity)) {
                const ComponentTypeId typeID = arr->GetTypeId();
                components.push_back(typeID);
            }
        }
        return components;
    }

    template<typename T>
    void RemoveComponent(const EntityID entity) {
        GetComponentArray<T>()->RemoveEntity(entity);
    }

    template<typename T>
    [[nodiscard]] bool HasComponent(const EntityID entity) const {
        const ComponentTypeId typeID = ComponentTypeHelper<T>::ID;
        const auto componentArray = std::static_pointer_cast<ComponentArray<T> >(m_ComponentArrays[typeID]);
        return componentArray->HasData(entity);
    }
};


#endif //GAME_ENGINE_COMPONENT_MANAGER_H
