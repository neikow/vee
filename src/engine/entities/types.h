#ifndef GAME_ENGINE_ENTITY_TYPES_H
#define GAME_ENGINE_ENTITY_TYPES_H
#include <cstdint>
#include <bitset>

namespace Entities {
    using EntityID = std::uint32_t;
    using Signature = std::bitset<8>;
}

#endif //GAME_ENGINE_ENTITY_TYPES_H
