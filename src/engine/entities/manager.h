#ifndef GAME_ENGINE_ENTITY_H
#define GAME_ENGINE_ENTITY_H
#include <bitset>
#include <iostream>
#include <set>
#include <vector>

#include "types.h"
#include "../utils/entity_utils.h"

using namespace Entities;

constexpr size_t MAX_ENTITIES = 10000;

constexpr size_t MAX_COMPONENTS = COMPONENTS_COUNT;

struct EntityData {
    EntityID id;
    std::string name;
};

class AvailableEntityQueue : public std::queue<EntityID> {
public:
    /** Removes a specific EntityID from the queue.
     *  This is an O(n) operation.
     */
    void pop_value(const EntityID entityID) {
        std::queue<EntityID> tempQueue;
        while (!this->empty()) {
            EntityID currentID = this->front();
            this->pop();
            if (currentID != entityID) {
                tempQueue.push(currentID);
            }
        }
        std::swap(*this, tempQueue);
    }
};

class EntityManager {
    AvailableEntityQueue m_AvailableEntityIDs;
    std::vector<Signature> m_Signatures;
    std::vector<EntityData> m_Entities;
    std::set<EntityID> m_ActiveEntities;

public:
    EntityManager() {
        for (EntityID offset = 0; offset < MAX_ENTITIES; ++offset) {
            m_AvailableEntityIDs.push(STARTING_ENTITY_ID + offset);
        }
        m_Signatures.resize(MAX_ENTITIES, 0);
        m_Entities.reserve(MAX_ENTITIES);
    }

    /** Creates a new entity with a unique ID.
     *  @param name The name of the new entity.
     *
     *  @return The EntityID of the newly created entity.
     */
    EntityID CreateEntity(const std::string &name) {
        const EntityID newID = m_AvailableEntityIDs.front();
        m_Entities.emplace_back(newID, name);
        m_Signatures[newID - STARTING_ENTITY_ID] = 0;
        m_ActiveEntities.insert(newID);
        m_AvailableEntityIDs.pop();
        return newID;
    }

    /** Creates a new entity with a custom ID.
     *
     *  This overload is mainly intended for deserialization purposes where speed is not critical.
     *
     *  @param name The name of the new entity.
     *  @param customEntityId The custom EntityID to assign to the new entity.
     *  @return The EntityID of the newly created entity.
     *  @throws std::runtime_error if the customEntityId is out of range or already in use.
     */
    EntityID CreateEntity(const std::string &name, const EntityID customEntityId) {
        // Sanity checks
        if (customEntityId < STARTING_ENTITY_ID || customEntityId >= STARTING_ENTITY_ID + MAX_ENTITIES) {
            throw std::runtime_error("Custom EntityID out of range: " + std::to_string(customEntityId));
        }
        if (m_ActiveEntities.contains(customEntityId)) {
            throw std::runtime_error("Custom EntityID already in use: " + std::to_string(customEntityId));
        }

        m_Entities.emplace_back(customEntityId, name);
        m_Signatures[customEntityId - STARTING_ENTITY_ID] = 0;
        m_ActiveEntities.insert(customEntityId);

        // Small optimization: if the custom ID is the next available one, just pop it.
        if (customEntityId == m_AvailableEntityIDs.front()) {
            m_AvailableEntityIDs.pop();
            return customEntityId;
        }

        // Otherwise, remove it from the available IDs queue. This is an O(n) operation.
        m_AvailableEntityIDs.pop_value(customEntityId);
        return customEntityId;
    }

    [[nodiscard]] Utils::Entities::EntityMatchRange GetEntitiesWithSignature(const Signature &signature) {
        return {&m_Signatures, signature};
    }

    void SetSignature(const EntityID entityID, const Signature signature) {
        m_Signatures[entityID - STARTING_ENTITY_ID] = signature;
    }

    [[nodiscard]] Signature GetSignature(const EntityID entityID) const {
        if (!m_ActiveEntities.contains(entityID)) {
            throw std::runtime_error(
                "Trying to get signature of non-active entity: " + std::to_string(entityID));
        }
        return m_Signatures[entityID - STARTING_ENTITY_ID];
    }

    [[nodiscard]] std::vector<EntityData> GetAllEntities() const {
        std::vector<EntityData> entities;
        entities.reserve(m_ActiveEntities.size());
        for (const EntityID entityID: m_ActiveEntities) {
            entities.push_back(m_Entities[entityID - STARTING_ENTITY_ID]);
        }
        return entities;
    }

    void RenameEntity(const EntityID entity, const std::string &newName) {
        m_Entities[entity - STARTING_ENTITY_ID].name = newName;
    }

    /** Deletes an entity, making its ID available for reuse.
     */
    void RemoveEntity(const EntityID entity) {
        m_AvailableEntityIDs.push(entity);
        m_Signatures[entity - STARTING_ENTITY_ID].reset();
        m_ActiveEntities.erase(entity);
    }

    /** Returns a reference to the name of the entity.
     */
    std::string &GetEntityName(const EntityID entity) {
        return m_Entities[entity - STARTING_ENTITY_ID].name;
    };
};

#endif //GAME_ENGINE_ENTITY_H
