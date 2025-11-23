#ifndef GAME_ENGINE_COMPONENT_MANAGER_H
#define GAME_ENGINE_COMPONENT_MANAGER_H
#include <unordered_map>

#include "../manager.h"
#include "../system/system_manager.h"

class EntityManager;

class IComponentArray {
public:
    virtual ~IComponentArray() = default;

    virtual void EntityDestroyed(EntityID entity) = 0;
};

template<typename T>
class ComponentArray final : public IComponentArray {
    std::vector<T> m_ComponentArray;

    std::unordered_map<EntityID, size_t> m_EntityToIndexMap;
    std::unordered_map<size_t, EntityID> m_IndexToEntityMap;

    size_t m_Size = 0;

public:
    void InsertData(const EntityID entity, T &component) {
        m_EntityToIndexMap[entity] = m_Size;
        m_IndexToEntityMap[m_Size] = entity;
        m_ComponentArray.push_back(component);
        m_Size++;
    }

    T &GetData(const EntityID entity) {
        return m_ComponentArray[m_EntityToIndexMap[entity]];
    }

    void EntityDestroyed(const EntityID entity) override {
        if (!m_EntityToIndexMap.contains(entity)) {
            return;
        }

        const size_t indexOfRemoved = m_EntityToIndexMap[entity];
        const EntityID entityToSwap = m_IndexToEntityMap[m_Size - 1];

        m_ComponentArray[indexOfRemoved] = m_ComponentArray[m_Size - 1];

        m_EntityToIndexMap[entityToSwap] = indexOfRemoved;
        m_IndexToEntityMap[indexOfRemoved] = entityToSwap;

        m_EntityToIndexMap.erase(entity);
        m_IndexToEntityMap.erase(m_Size - 1);

        m_Size--;
    };
};

class ComponentManager {
    std::shared_ptr<EntityManager> m_EntityManager;
    std::shared_ptr<SystemManager> m_SystemManager;
    std::array<std::shared_ptr<IComponentArray>, MAX_COMPONENTS> m_ComponentArrays;
    std::unordered_map<const char *, ComponentTypeID> m_ComponentTypeMap;

    template<typename T>
    std::shared_ptr<ComponentArray<T> > GetComponentArray() {
        const ComponentTypeID typeID = ComponentTypeHelper<T>::ID;
        return std::static_pointer_cast<ComponentArray<T> >(m_ComponentArrays[typeID]);
    }

public:
    explicit ComponentManager(
        const std::shared_ptr<SystemManager> &systemManager,
        const std::shared_ptr<EntityManager> &entityManager
    ) : m_EntityManager(entityManager), m_SystemManager(systemManager) {
    }

    template<typename T>
    void RegisterComponent() {
        const char *typeName = typeid(T).name();
        ComponentTypeID typeID = ComponentTypeHelper<T>::ID;
        m_ComponentTypeMap.insert({typeName, typeID});
        m_ComponentArrays[typeID] = std::make_shared<ComponentArray<T> >();
    }

    template<typename T>
    void AddComponent(EntityID entity, T component) {
        GetComponentArray<T>()->InsertData(entity, component);

        Signature signature = m_EntityManager->GetSignature(entity);
        const ComponentTypeID typeID = ComponentTypeHelper<T>::ID;

        signature.set(typeID);

        m_EntityManager->SetSignature(entity, signature);
        m_SystemManager->EntitySignatureChanged(entity, signature);
    }

    template<typename T>
    T &GetComponent(EntityID entity) {
        return GetComponentArray<T>()->GetData(entity);
    }

    void EntityDestroyed(const EntityID entity) {
        for (auto const &arr: m_ComponentArrays) {
            if (arr) {
                arr->EntityDestroyed(entity);
            }
        }
    }
};


#endif //GAME_ENGINE_COMPONENT_MANAGER_H
