#ifndef GAME_ENGINE_ENTITY_COMPONENT_BASE_H
#define GAME_ENGINE_ENTITY_COMPONENT_BASE_H
#include <cstdint>

enum ComponentType {
    CAMERA_COMPONENT,
    TRANSFORM_COMPONENT,
    VELOCITY_COMPONENT,
    RADIAL_VELOCITY_COMPONENT,
    MESH_COMPONENT,
    TEXTURE_COMPONENT,
    // TODO: Add more components
    COUNT,
};

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
