#ifndef VEE_COMPONENT_ARRAY_H
#define VEE_COMPONENT_ARRAY_H
#include <unordered_map>
#include <vector>

#include "../types.h"

namespace Entities {
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
            m_ComponentArray
                    .
                    push_back(component);
            m_Size
                    ++;
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
        }
    };
}

#endif //VEE_COMPONENT_ARRAY_H
