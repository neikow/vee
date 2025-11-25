#ifndef GAME_ENGINE_ENTITY_COMPONENT_BASE_H
#define GAME_ENGINE_ENTITY_COMPONENT_BASE_H
#include <cstdint>

// Define component types with unique integer values
// to ensure consistent ordering and identification.
// Update COMPONENTS_COUNT when adding new component types.
enum class ComponentType : uint16_t {
    CAMERA = 0,
    TRANSFORM = 10,
    VELOCITY = 20,
    RENDERABLE = 30,
    ACTIVE_CAMERA_TAG = 40,
};

// Total number of distinct component types defined.
// This value should match the number of entries in the ComponentType enum.
constexpr uint16_t COMPONENTS_COUNT = 5;

using ComponentTypeID = uint8_t;

inline ComponentTypeID GetNewComponentTypeID() {
    static ComponentTypeID lastID = 0;
    return lastID++;
}

template<typename T>
class ComponentTypeHelper {
public:
    static const ComponentTypeID ID;
};

template<typename T>
const ComponentTypeID ComponentTypeHelper<T>::ID = GetNewComponentTypeID();


#endif //GAME_ENGINE_ENTITY_COMPONENT_BASE_H
