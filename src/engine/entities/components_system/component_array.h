#ifndef VEE_COMPONENT_ARRAY_H
#define VEE_COMPONENT_ARRAY_H
#include <unordered_map>
#include <vector>

#include "../types.h"
#include "component_base.h"

namespace Entities {
    class IComponentArray {
    public:
        [[nodiscard]] virtual ComponentTypeId GetTypeId() const = 0;

        [[nodiscard]] virtual bool HasData(EntityID entity) const = 0;

        virtual void InsertDefault(EntityID entity) = 0;

        virtual ~IComponentArray() = default;

        virtual void RemoveEntity(EntityID entity) = 0;
    };


    template<typename T>
    class ComponentArray final : public IComponentArray {
        std::vector<T> m_ComponentArray;

        ComponentTypeId m_TypeID;
        std::unordered_map<EntityID, size_t> m_EntityToIndexMap;
        std::unordered_map<size_t, EntityID> m_IndexToEntityMap;

        size_t m_Size = 0;

        T m_Default{};

        /** Inserts a component for the given entity.
        * Internal use only.
        */
        void InternalInsert(const EntityID entity, const T &component) {
            m_EntityToIndexMap[entity] = m_Size;
            m_IndexToEntityMap[m_Size] = entity;
            m_ComponentArray.push_back(component);
            m_Size++;
        }

        /** Swaps the component data between two entities.
        * Used when an entity is removed and the last entity's data is moved to its place.
        * Internal use only.
        */
        void InternalSwapEntityDataSwapEntityData(const EntityID entityA, const EntityID entityB) {
            const auto entityAIndex = m_EntityToIndexMap[entityA],
                    entityBIndex = m_EntityToIndexMap[entityB];

            std::swap(
                m_ComponentArray[entityAIndex],
                m_ComponentArray[entityBIndex]
            );

            m_EntityToIndexMap[entityA] = entityBIndex;
            m_EntityToIndexMap[entityB] = entityAIndex;
            m_IndexToEntityMap[entityAIndex] = entityB;
            m_IndexToEntityMap[entityBIndex] = entityA;
        }

    public:
        ComponentArray() {
            m_TypeID = ComponentTypeHelper<T>::ID;
        }

        /** Returns the type ID of the component stored in this array.
        */
        [[nodiscard]] ComponentTypeId GetTypeId() const override {
            return m_TypeID;
        }

        /** Checks if the given entity has a component stored in this array.
        */
        [[nodiscard]] bool HasData(const EntityID entity) const override {
            return m_EntityToIndexMap.contains(entity);
        }

        /** Inserts a component for the given entity.
        */
        void InsertData(const EntityID entity, const T &component) {
            InternalInsert(entity, component);
        }

        /** Inserts a default-initialized component for the given entity.
        */
        void InsertDefault(const EntityID entity) override {
            const auto defaultCopy = m_Default;
            InternalInsert(entity, defaultCopy);
        }

        /** Retrieves a reference to the component data for the given entity.
        */
        T &GetData(const EntityID entity) {
            return m_ComponentArray[m_EntityToIndexMap[entity]];
        }

        /** Removes the component data for the given entity.
        */
        void RemoveEntity(const EntityID entity) override {
            if (entity == NULL_ENTITY) {
                throw std::runtime_error("Trying to remove NULL_ENTITY from ComponentArray");
            }

            if (!m_EntityToIndexMap.contains(entity)) {
                return;
            }

            const size_t indexOfLastElement = m_Size - 1;
            const EntityID entityOfLastElement = m_IndexToEntityMap[indexOfLastElement];

            InternalSwapEntityDataSwapEntityData(entity, entityOfLastElement);
            m_EntityToIndexMap.erase(entity);
            m_IndexToEntityMap.erase(indexOfLastElement);
            m_ComponentArray.pop_back();

            m_Size--;
        }
    };
}

#endif //VEE_COMPONENT_ARRAY_H
