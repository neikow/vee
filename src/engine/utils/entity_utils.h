#ifndef GAME_ENGINE_ENTITY_UTILS_H
#define GAME_ENGINE_ENTITY_UTILS_H
#include <iterator>
#include <cstddef>
#include <iostream>
#include <vector>
#include "../entities/types.h"

using namespace Entities;

namespace Utils::Entities {
    // TODO: Move me to utils/entities/iteration.h
    struct EntityMatchIterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type = EntityID;
        using difference_type = std::ptrdiff_t;
        using pointer = const EntityID *;
        using reference = const EntityID &;

    private:
        const std::vector<Signature> *m_Signatures = nullptr;
        Signature m_Query;
        std::size_t m_Index = 0;
        EntityID m_Current = 0;

        void advance_to_match() {
            const auto n = m_Signatures->size();
            while (m_Index < n) {
                if (((*m_Signatures)[m_Index] & m_Query) == m_Query) {
                    m_Current = static_cast<EntityID>(m_Index + STARTING_ENTITY_ID);
                    return;
                }
                ++m_Index;
            }
            m_Index = n;
        }

    public:
        EntityMatchIterator() = default;

        EntityMatchIterator(const std::vector<Signature> *signs, Signature query, std::size_t start = 0)
            : m_Signatures(signs), m_Query(query), m_Index(start) {
            if (m_Signatures) advance_to_match();
        }

        value_type operator*() const { return m_Current; }

        EntityMatchIterator &operator++() {
            ++m_Index;
            advance_to_match();
            return *this;
        }

        EntityMatchIterator operator++(int) {
            const EntityMatchIterator tmp = *this;
            ++*this;
            return tmp;
        }

        bool operator==(const EntityMatchIterator &o) const {
            return m_Signatures == o.m_Signatures && m_Index == o.m_Index && m_Query == o.m_Query;
        }

        bool operator!=(const EntityMatchIterator &o) const { return !(*this == o); }
    };

    struct EntityMatchRange {
        std::vector<Signature> *m_Signatures;
        Signature m_Query;

        EntityMatchRange(
            std::vector<Signature> *signs,
            const Signature query
        )
            : m_Signatures(signs), m_Query(query) {
        }

        [[nodiscard]] EntityMatchIterator begin() const {
            return {m_Signatures, m_Query, 0};
        }

        [[nodiscard]] EntityMatchIterator end() const {
            return {m_Signatures, m_Query, m_Signatures ? m_Signatures->size() : 0};
        }
    };

    inline EntityID GetFirstEntityWithSignature(
        const EntityMatchRange &entityRange
    ) {
        for (const auto entityID: entityRange) {
            return entityID;
        }
        return NULL_ENTITY;
    }
}
#endif //GAME_ENGINE_ENTITY_UTILS_H
