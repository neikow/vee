#ifndef GAME_ENGINE_ENTITY_H
#define GAME_ENGINE_ENTITY_H
#include <bitset>
#include <iostream>
#include <queue>
#include <vector>

#include "types.h"
#include "components_system/component_base.h"
#include "../utils/entity_utils.h"

using namespace Entities;

constexpr EntityID MAX_ENTITIES = 1'000;

constexpr size_t MAX_COMPONENTS = COMPONENTS_COUNT;

struct EntityData {
    EntityID id;
};

class EntityManager {
    std::vector<Signature> m_Signatures;
    std::queue<EntityID> m_AvailableEntities;
    EntityID m_LivingEntityCount = 0;

public:
    EntityManager() {
        m_Signatures.reserve(MAX_ENTITIES);
        for (EntityID i = STARTING_ENTITY_ID; i < MAX_ENTITIES + STARTING_ENTITY_ID; ++i) {
            m_AvailableEntities.push(i);
            m_Signatures.emplace_back();
        }
    }

    EntityID CreateEntity() {
        const EntityID newID = m_AvailableEntities.front();
        m_AvailableEntities.pop();
        m_Signatures[newID] = 0;
        m_LivingEntityCount++;
        return newID;
    }

    [[nodiscard]] Utils::Entities::EntityMatchRange GetEntitiesWithSignature(const Signature &signature) {
        return {&m_Signatures, signature};
    }

    void SetSignature(const EntityID entityID, const Signature signature) {
        m_Signatures[entityID - STARTING_ENTITY_ID] = signature;
    }

    [[nodiscard]] Signature GetSignature(const EntityID entityID) const {
        return m_Signatures[entityID - STARTING_ENTITY_ID];
    }

    [[nodiscard]] std::vector<EntityData> GetAllEntities() const {
        auto entities = std::vector<EntityData>{};
        for (EntityID i = STARTING_ENTITY_ID; i < m_LivingEntityCount + STARTING_ENTITY_ID; ++i) {
            entities.push_back({i});
        }
        return entities;
    }
};

#endif //GAME_ENGINE_ENTITY_H
