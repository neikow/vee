#ifndef GAME_ENGINE_ENTITY_TYPES_H
#define GAME_ENGINE_ENTITY_TYPES_H
#include <cstdint>
#include <bitset>

// Define component types with unique integer values
// to ensure consistent ordering and identification.
// Update COMPONENTS_COUNT when adding new component types.
enum class ComponentType : uint16_t {
    INTERNAL = 0,
    PARENT = 1,
    CHILDREN = 2,
    CAMERA = 10,
    LOCAL_TO_WORLD = 15,
    LOCAL_TRANSFORM = 20,
    VELOCITY = 30,
    RENDERABLE = 40,
    ACTIVE_CAMERA_TAG = 50,
    EDITOR_CAMERA_TAG = 1000,
};

// Total number of distinct component types defined (Components + Tags).
// This value should match the number of entries in the ComponentType enum.
constexpr uint16_t COMPONENTS_COUNT = 10;

namespace Entities {
    using EntityID = std::uint32_t;
    using Signature = std::bitset<COMPONENTS_COUNT>;

    inline EntityID NULL_ENTITY = 0;
    inline EntityID STARTING_ENTITY_ID = 1;
}

#endif //GAME_ENGINE_ENTITY_TYPES_H
