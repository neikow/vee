#ifndef GAME_ENGINE_ENTITY_COMPONENT_BASE_H
#define GAME_ENGINE_ENTITY_COMPONENT_BASE_H
#include <cstdint>

using ComponentTypeId = uint8_t;

inline ComponentTypeId GetNewComponentTypeID() {
    static ComponentTypeId lastID = 0;
    return lastID++;
}

template<typename T>
class ComponentTypeHelper {
public:
    static const ComponentTypeId ID;
};

template<typename T>
const ComponentTypeId ComponentTypeHelper<T>::ID = GetNewComponentTypeID();


#endif //GAME_ENGINE_ENTITY_COMPONENT_BASE_H
