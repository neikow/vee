#ifndef GAME_ENGINE_ENTITY_H
#define GAME_ENGINE_ENTITY_H
#include <bitset>
#include <iostream>
#include <set>
#include <vector>

#include "types.h"
#include "components_system/component_base.h"
#include "../utils/entity_utils.h"

using namespace Entities;

constexpr size_t MAX_ENTITIES = 10000;

constexpr size_t MAX_COMPONENTS = COMPONENTS_COUNT;

struct EntityData {
    EntityID id;
    std::string name;
};

class EntityManager {
    std::queue<EntityID> m_AvailableEntityIDs;
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

    EntityID CreateEntity(const std::string &name = "") {
        const EntityID newID = m_AvailableEntityIDs.front();
        m_Entities.emplace_back(newID, name);
        m_Signatures[newID - STARTING_ENTITY_ID] = 0;
        m_ActiveEntities.insert(newID);
        m_AvailableEntityIDs.pop();
        return newID;
    }

    [[nodiscard]] Utils::Entities::EntityMatchRange GetEntitiesWithSignature(const Signature &signature) {
        return {&m_Signatures, signature};
    }

    void SetSignature(const EntityID entityID, const Signature signature) {
        m_Signatures[entityID - STARTING_ENTITY_ID] = signature;
    }

    [[nodiscard]] Signature GetSignature(const EntityID entityID) const {
        if (!m_ActiveEntities.contains(entityID)) {
            throw std::runtime_error("Trying to get signature of non-active entity: " + std::to_string(entityID));
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
