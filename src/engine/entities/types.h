#ifndef GAME_ENGINE_ENTITY_TYPES_H
#define GAME_ENGINE_ENTITY_TYPES_H
#include <cstdint>
#include <bitset>

// Define component types with unique integer values
// to ensure consistent ordering and identification.
// Update COMPONENTS_COUNT when adding new component types.
constexpr auto VEE_INTERNAL_COMPONENT_NAME = "InternalTagComponent";
constexpr auto VEE_PHYSICS_SETTINGS_COMPONENT_NAME = "PhysicsSettings";
constexpr auto VEE_PARENT_COMPONENT_NAME = "ParentComponent";
constexpr auto VEE_CHILDREN_COMPONENT_NAME = "ChildrenComponent";
constexpr auto VEE_CAMERA_COMPONENT_NAME = "CameraComponent";
constexpr auto VEE_LOCAL_TO_WORLD_COMPONENT_NAME = "LocalToWorldComponent";
constexpr auto VEE_LOCAL_TRANSFORM_COMPONENT_NAME = "LocalTransformComponent";
constexpr auto VEE_VELOCITY_COMPONENT_NAME = "VelocityComponent";
constexpr auto VEE_RENDERABLE_COMPONENT_NAME = "RenderableComponent";
constexpr auto VEE_PLAYER_CONTROLLER_COMPONENT_NAME = "PlayerControllerComponent";
constexpr auto VEE_ACTIVE_CAMERA_TAG_COMPONENT_NAME = "ActiveCameraTagComponent";
constexpr auto VEE_EDITOR_CAMERA_TAG_COMPONENT_NAME = "EditorCameraTagComponent";

// Total number of distinct component types defined (Components + Tags).
// This value should match the number of entries in the ComponentType enum.
constexpr uint16_t COMPONENTS_COUNT = 12;

namespace Entities {
    using EntityID = std::uint32_t;
    using Signature = std::bitset<COMPONENTS_COUNT>;

    inline EntityID NULL_ENTITY = 0;
    inline EntityID STARTING_ENTITY_ID = 1;
}

#endif //GAME_ENGINE_ENTITY_TYPES_H
