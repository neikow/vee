#include "scene_serializer.h"

#include <fstream>

#include "../engine.h"
#include "../entities/components_system/components/camera_component.h"
#include "../entities/components_system/components/local_transform_component.h"
#include "../entities/components_system/components/children_component.h"
#include "../entities/components_system/components/local_to_world_component.h"
#include "../entities/components_system/components/parent_component.h"
#include "../entities/components_system/components/player_controller_component.h"
#include "../entities/components_system/components/renderable_component.h"
#include "../entities/components_system/components/velocity_component.h"
#include "../entities/components_system/tags/active_camera_tag_component.h"
#include "../utils/entities/hierarchy.h"
#include "../utils/macros/log_macros.h"
#include "yaml-cpp/yaml.h"

struct InternalTagComponent;

void operator>>(const YAML::Node &node, glm::vec3 &v) {
    v.x = node[0].as<float>();
    v.y = node[1].as<float>();
    v.z = node[2].as<float>();
}

void operator>>(const YAML::Node &node, glm::quat &q) {
    q = glm::normalize(
        glm::quat(
            node[0].as<float>(),
            node[1].as<float>(),
            node[2].as<float>(),
            node[3].as<float>()
        )
    );
}

void operator>>(const YAML::Node &node, LocalTransformComponent &c) {
    node["position"] >> c.position;
    node["rotation"] >> c.rotation;
    node["scale"] >> c.scale;
}

void operator>>(const YAML::Node &node, float &f) {
    f = node.as<float>();
}

void operator>>(const YAML::Node &node, uint32_t &f) {
    f = node.as<uint32_t>();
}

void operator>>(const YAML::Node &node, ProjectionType &pt) {
    const auto val = node.as<std::string>();
    if (val == "perspective") {
        pt = PERSPECTIVE;
    } else if (val == "orthographic") {
        pt = ORTHOGRAPHIC;
    }
}

void operator>>(const YAML::Node &node, CameraComponent &c) {
    node["projection"] >> c.projection;
    node["fov"] >> c.fieldOfView;
    node["aspect_ratio"] >> c.aspectRatio;
    node["near_plane"] >> c.nearPlane;
    node["far_plane"] >> c.farPlane;

    if (c.projection == ORTHOGRAPHIC) {
        node["ortho_scale"] >> c.orthoScale;
    }
}

void operator>>(const YAML::Node &node, VelocityComponent &c) {
    node["linear"] >> c.linearVelocity;
    node["angular"] >> c.angularVelocity;
}

void operator>>(const YAML::Node &node, RenderableComponent &c) {
    node["mesh_id"] >> c.meshId;
    node["texture_id"] >> c.textureId;
}

void operator>>(const YAML::Node &node, ParentComponent &c) {
    node["parent_id"] >> c.parent;
}

void operator>>(const YAML::Node &node, Key &key) {
    key = static_cast<Key>(node.as<int>());
}

void operator>>(const YAML::Node &node, PlayerControllerComponent &c) {
    node["movement_speed"] >> c.movementSpeed;
    node["forward_direction"] >> c.forwardDirection;
    c.forwardDirection = glm::normalize(c.forwardDirection);
    node["move_forward_key"] >> c.moveForwardKey;
    node["move_backward_key"] >> c.moveBackwardKey;
    node["move_left_key"] >> c.moveLeftKey;
    node["move_right_key"] >> c.moveRightKey;
}

void DeserializeComponentV0(
    const YAML::Node &componentNode,
    const EntityID entityId,
    const std::unique_ptr<Scene> &scenePtr
) {
    const auto componentType = componentNode["type"];
    if (!componentType) {
        throw std::runtime_error("Invalid component node");
    }

    const auto typeStr = componentType.as<std::string>();
    const auto componentManager = scenePtr->GetComponentManager();

    if (typeStr == VEE_PARENT_COMPONENT_NAME) {
        ParentComponent parent{};
        componentNode >> parent;
        Utils::Entities::Hierarchy::SetParent(entityId, parent.parent, componentManager);
    } else if (typeStr == VEE_LOCAL_TRANSFORM_COMPONENT_NAME) {
        LocalTransformComponent transform{};
        componentNode >> transform;
        componentManager->AddComponent<LocalTransformComponent>(entityId, transform);
        componentManager->AddComponent<LocalToWorldComponent>(entityId, {});
    } else if (typeStr == VEE_CAMERA_COMPONENT_NAME) {
        CameraComponent camera{};
        componentNode >> camera;
        componentManager->AddComponent<CameraComponent>(entityId, camera);
    } else if (typeStr == VEE_ACTIVE_CAMERA_TAG_COMPONENT_NAME) {
        constexpr ActiveCameraTagComponent tag{};
        componentManager->AddComponent<ActiveCameraTagComponent>(entityId, tag);
    } else if (typeStr == VEE_VELOCITY_COMPONENT_NAME) {
        VelocityComponent velocity{};
        componentNode >> velocity;
        componentManager->AddComponent<VelocityComponent>(entityId, velocity);
    } else if (typeStr == VEE_RENDERABLE_COMPONENT_NAME) {
        RenderableComponent renderable{};
        componentNode >> renderable;
        componentManager->AddComponent<RenderableComponent>(entityId, renderable);
    } else if (typeStr == VEE_PLAYER_CONTROLLER_COMPONENT_NAME) {
        PlayerControllerComponent playerController{};
        componentNode >> playerController;
        componentManager->AddComponent<PlayerControllerComponent>(entityId, playerController);
    } else {
        throw std::runtime_error("Unknown component type: " + typeStr);
    }
};

void DeserializeEntityV0(
    const YAML::Node &entityNode, const std::unique_ptr<Scene> &scenePtr) {
    std::string entityName;
    if (entityNode["name"]) {
        entityName = entityNode["name"].as<std::string>();
    }
    if (!entityNode["id"]) {
        throw std::runtime_error("Invalid entity node: missing 'id'");
    }

    const auto entityId = scenePtr->CreateEntity(
        entityName,
        entityNode["id"].as<EntityID>()
    );
    const auto components = entityNode["components"];
    if (!components) {
        throw std::runtime_error("Invalid entity node");
    }

    for (const auto &component: components) {
        DeserializeComponentV0(
            component, entityId, scenePtr
        );
    }
};

void DeserializeSceneV0(
    const YAML::Node &data,
    const std::unique_ptr<Scene> &scenePtr
) {
    if (const auto entities = data["entities"]) {
        for (auto entityNode: entities) {
            DeserializeEntityV0(entityNode, scenePtr);
        }
    } else {
        std::cerr << "[WARN] No entities found in scene." << std::endl;
    }
    if (const auto meshes = data["meshes"]) {
        const auto renderer = scenePtr->GetRenderer();
        for (auto meshNode: meshes) {
            const auto meshPath = meshNode["path"].as<std::string>();
            renderer->GetMeshManager()->LoadMesh(meshPath);
        }
    } else {
        std::cerr << "[WARN] No meshes found in scene data." << std::endl;
    }
    if (const auto textures = data["textures"]) {
        const auto renderer = scenePtr->GetRenderer();
        for (auto textureNode: textures) {
            const auto textureId = textureNode["id"].as<TextureId>();
            const auto texturePath = textureNode["path"].as<std::string>();
            renderer->GetTextureManager()->LoadTexture(textureId, texturePath);
        }
    } else {
        std::cerr << "[WARN] No textures found in scene data." << std::endl;
    }
}

std::unique_ptr<Scene> SceneSerializer::LoadScene(
    const std::string &scenePath,
    const std::shared_ptr<AbstractRenderer> &renderer,
    const std::vector<SystemRegistrationFunction> &systemRegistrations
) {
    auto scene = std::make_unique<Scene>(scenePath, renderer, systemRegistrations);

    YAML::Node data;
    try {
        data = YAML::LoadFile(scenePath);
    } catch (YAML::ParserException &e) {
        throw std::runtime_error("Failed to load scene from YAML, error: " + std::string(e.what()));
    }

    if (data["type"].as<std::string>() != "scene") {
        throw std::runtime_error("Invalid scene file: missing 'scene' type");
    }

    if (data["name"]) {
        scene->SetName(data["name"].as<std::string>());
    }

    if (!data["version"]) {
        throw std::runtime_error("Invalid scene file: missing 'version'");
    }

    switch (
        static_cast<SceneVersion>(data["version"].as<SceneVersionRaw>())
    ) {
        case SceneVersion::V_0:
            DeserializeSceneV0(data, scene);
            break;
        default:
            throw std::runtime_error("Invalid scene version");
    }


    return scene;
}

void operator<<(YAML::Emitter &out, const glm::vec3 &v) {
    out << YAML::Flow << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
}

void operator<<(YAML::Emitter &out, const glm::quat &q) {
    out << YAML::Flow << YAML::BeginSeq << q.w << q.x << q.y << q.z << YAML::EndSeq;
}

void operator<<(YAML::Emitter &out, const LocalTransformComponent &transform) {
    out << YAML::Key << "position" << YAML::Value << transform.position;
    out << YAML::Key << "rotation" << YAML::Value << transform.rotation;
    out << YAML::Key << "scale" << YAML::Value << transform.scale;
}

void operator<<(YAML::Emitter &out, const CameraComponent &camera) {
    out << YAML::Key << "projection" << YAML::Value
            << (camera.projection == PERSPECTIVE ? "perspective" : "orthographic");
    out << YAML::Key << "fov" << YAML::Value << camera.fieldOfView;
    out << YAML::Key << "aspect_ratio" << YAML::Value << camera.aspectRatio;
    out << YAML::Key << "near_plane" << YAML::Value << camera.nearPlane;
    out << YAML::Key << "far_plane" << YAML::Value << camera.farPlane;
}

void operator<<(YAML::Emitter &out, const VelocityComponent &velocity) {
    out << YAML::Key << "linear" << YAML::Value << velocity.linearVelocity;
    out << YAML::Key << "angular" << YAML::Value << velocity.angularVelocity;
}

void operator<<(YAML::Emitter &out, const RenderableComponent &renderable) {
    out << YAML::Key << "mesh_id" << YAML::Value << renderable.meshId;
    out << YAML::Key << "texture_id" << YAML::Value << renderable.textureId;
}

void operator<<(YAML::Emitter &out, const ParentComponent &parent) {
    out << YAML::Key << "parent_id" << YAML::Value << parent.parent;
}

void SerializeSceneV0(
    YAML::Emitter &out,
    const std::shared_ptr<Scene> &scene
) {
    out << YAML::Newline;
    out << YAML::Newline;
    scene->GetRenderer()->GetMeshManager()->DumpLoadedMeshes(out);
    out << YAML::Newline;
    out << YAML::Newline;
    scene->GetRenderer()->GetTextureManager()->DumpLoadedTextures(out);
    out << YAML::Newline;
    out << YAML::Newline;

    out << YAML::Key << "entities" << YAML::Value << YAML::BeginSeq;
    const auto entityManager = scene->GetEntityManager();
    const auto componentManager = scene->GetComponentManager();

    for (auto &[entity, name]: entityManager->GetAllEntities()) {
        if (componentManager->HasComponent<InternalTagComponent>(entity)) {
            // Skip internal entities
            continue;
        }

        out << YAML::BeginMap;
        out << YAML::Key << "id" << YAML::Value << entity;
        out << YAML::Key << "name" << YAML::Value << name;

        out << YAML::Key << "components" << YAML::Value << YAML::BeginSeq;

        const auto componentTypes = componentManager->GetEntityComponents(entity);
        for (const auto &componentType: componentTypes) {
            const auto componentTypeName = componentManager->GetComponentName(componentType);

            if (
                componentType == ComponentTypeHelper<LocalToWorldComponent>::ID
                || componentType == ComponentTypeHelper<ChildrenComponent>::ID
            ) {
                // This component is derived, no need to serialize
                continue;
            }

            out << YAML::BeginMap;
            out << YAML::Key << "type" << YAML::Value << componentTypeName;

            if (componentType == ComponentTypeHelper<ParentComponent>::ID) {
                out << componentManager->GetComponent<ParentComponent>(entity);
            } else if (componentType == ComponentTypeHelper<LocalTransformComponent>::ID) {
                out << componentManager->GetComponent<LocalTransformComponent>(entity);
            } else if (componentType == ComponentTypeHelper<CameraComponent>::ID) {
                out << componentManager->GetComponent<CameraComponent>(entity);
            } else if (componentType == ComponentTypeHelper<VelocityComponent>::ID) {
                out << componentManager->GetComponent<VelocityComponent>(entity);
            } else if (componentType == ComponentTypeHelper<RenderableComponent>::ID) {
                out << componentManager->GetComponent<RenderableComponent>(entity);
            } else if (componentType == ComponentTypeHelper<ActiveCameraTagComponent>::ID) {
                // Tag component, no data to serialize
            } else {
                LOG_ERROR("Unknown component type during serialization: " + componentTypeName);
            }
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
        out << YAML::EndMap;
    }
}

void SceneSerializer::SaveScene(
    const std::string &scenePath,
    const std::shared_ptr<Scene> &scene,
    SceneVersion version
) {
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "type" << YAML::Value << "scene";
    out << YAML::Key << "version" << YAML::Value << static_cast<SceneVersionRaw>(version);
    out << YAML::Key << "name" << YAML::Value << scene->GetName();

    switch (version) {
        case SceneVersion::V_0:
            SerializeSceneV0(out, scene);
            break;
        default:
            throw std::runtime_error("Invalid scene version");
    }

    out << YAML::EndMap;

    std::ofstream fout(scenePath);

    if (!fout.is_open()) {
        throw std::runtime_error("Failed to open scene file for writing: " + scenePath);
    }

    fout << out.c_str();
}
