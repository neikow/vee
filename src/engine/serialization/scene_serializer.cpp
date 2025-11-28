#include "scene_serializer.h"

#include "../entities/components_system/components/camera_component.h"
#include "../entities/components_system/components/transform_component.h"
#include "../entities/components_system/components/camera_component.h"
#include "../entities/components_system/components/renderable_component.h"
#include "../entities/components_system/components/velocity_component.h"
#include "../entities/components_system/tags/active_camera_tag_component.h"
#include "yaml-cpp/yaml.h"

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

void operator>>(const YAML::Node &node, TransformComponent &c) {
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

void DeserializeComponentV0(
    const YAML::Node &componentNode,
    const EntityID entityId,
    const std::unique_ptr<Scene> &scenePtr
) {
    const auto componentType = componentNode["type"];
    if (!componentType) {
        throw std::runtime_error("Invalid component node");
    }

    const std::string typeStr = componentType.as<std::string>();
    const auto componentManager = scenePtr->GetComponentManager();

    if (typeStr == "TransformComponent") {
        TransformComponent transform{};
        componentNode >> transform;
        componentManager->AddComponent<TransformComponent>(entityId, transform);
    } else if (typeStr == "CameraComponent") {
        CameraComponent camera{};
        componentNode >> camera;
        componentManager->AddComponent<CameraComponent>(entityId, camera);
    } else if (typeStr == "ActiveCameraTagComponent") {
        constexpr ActiveCameraTagComponent tag{};
        componentManager->AddComponent<ActiveCameraTagComponent>(entityId, tag);
    } else if (typeStr == "VelocityComponent") {
        VelocityComponent velocity{};
        componentNode >> velocity;
        componentManager->AddComponent<VelocityComponent>(entityId, velocity);
    } else if (typeStr == "RenderableComponent") {
        RenderableComponent renderable{};
        componentNode >> renderable;
        componentManager->AddComponent<RenderableComponent>(entityId, renderable);
    } else {
        throw std::runtime_error("Unknown component type: " + typeStr);
    }
};

void DeserializeEntityV0(
    const YAML::Node &entityNode, const std::unique_ptr<Scene> &scenePtr) {
    const auto entityId = scenePtr->CreateEntity();
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
    }
    if (const auto meshes = data["meshes"]) {
        const auto renderer = scenePtr->GetRenderer();
        for (auto meshNode: meshes) {
            const auto meshPath = meshNode["path"].as<std::string>();
            renderer->GetMeshManager()->LoadModel(meshPath);
        }
    }
    if (const auto textures = data["textures"]) {
        const auto renderer = scenePtr->GetRenderer();
        for (auto textureNode: textures) {
            const auto textureId = textureNode["id"].as<TextureId>();
            const auto texturePath = textureNode["path"].as<std::string>();
            renderer->GetTextureManager()->LoadTexture(textureId, texturePath);
        }
    }
}

std::unique_ptr<Scene> SceneSerializer::LoadScene(
    const std::string &scenePath,
    const std::shared_ptr<AbstractRenderer> &renderer
) {
    auto scene = std::make_unique<Scene>(renderer);

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
