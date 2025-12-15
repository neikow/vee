#include "inspector.h"

#include "../../../engine/entities/components_system/components/camera_component.h"
#include "../../../engine/entities/components_system/components/children_component.h"
#include "../../../engine/entities/components_system/components/local_to_world_component.h"
#include "../../../engine/entities/components_system/components/local_transform_component.h"
#include "../../../engine/entities/components_system/components/parent_component.h"
#include "../../../engine/entities/components_system/components/physics_settings_component.h"
#include "../../../engine/entities/components_system/components/player_controller_component.h"
#include "../../../engine/entities/components_system/components/renderable_component.h"
#include "../../../engine/entities/components_system/components/velocity_component.h"
#include "../../../engine/entities/components_system/tags/active_camera_tag_component.h"
#include "../../../engine/logging/logger.h"
#include "../../../engine/utils/entities/hierarchy.h"

void SceneNameInput(const std::shared_ptr<Scene> &scene) {
    const auto currentSceneName = scene->GetName();
    char sceneNameBuffer[256] = "";
    std::strncpy(sceneNameBuffer, currentSceneName.c_str(), sizeof(sceneNameBuffer));
    ImGui::InputText("Name", sceneNameBuffer, IM_ARRAYSIZE(sceneNameBuffer));

    if (const auto sceneName = scene->GetName(); sceneName != std::string(sceneNameBuffer)) {
        scene->SetName(std::string(sceneNameBuffer));
    }
}

void Editor::UI::Inspector::DrawSceneInspector(const VeeEditor *editor, const std::shared_ptr<Scene> &scene) {
    SceneNameInput(scene);

    ImGui::Text("DEBUG: Current camera entityID : %d", editor->GetEngine()->GetActiveCameraEntityId());
}

void Editor::UI::Inspector::DrawEntityInspector(
    VeeEditor *editor,
    const std::shared_ptr<Scene> &scene,
    EntityID entity
) {
    const auto componentManager = scene->GetComponentManager();
    const auto entityManager = scene->GetEntityManager();
    const auto entityComponents = componentManager->GetEntityComponents(entity);
    const auto isInternal = componentManager->HasComponent<InternalTagComponent>(entity);

    const std::string entityNameString = entityManager->GetEntityName(entity);
    char entityName[256];
    std::strncpy(entityName, entityNameString.c_str(), sizeof(entityName));
    ImGui::InputText(
        "Name",
        entityName,
        IM_ARRAYSIZE(entityName)
    );
    if (const auto newName = std::string(entityName); newName != entityNameString) {
        entityManager->RenameEntity(
            entity,
            newName
        );
    }

    ImGui::Separator();

    const auto showDebugInfo = editor->GetEditorSettings().displayDebugInfo;

    for (const auto &componentTypeID: entityComponents) {
        constexpr ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;

        if (componentTypeID == ComponentTypeHelper<LocalTransformComponent>::ID) {
            if (ImGui::CollapsingHeader("Transform", flags)) {
                auto &transform = componentManager->GetComponent<LocalTransformComponent>(entity);

                ImGui::DragFloat3(
                    "Position",
                    &transform.position.x,
                    0.1f
                );

                auto rotation = glm::eulerAngles(transform.rotation) * 180.0f / glm::pi<float>();
                ImGui::DragFloat3(
                    "Rotation",
                    &rotation.x,
                    1.0f,
                    -360.0f,
                    360.0f
                );

                transform.rotation = glm::quat(rotation * glm::pi<float>() / 180.0f);

                ImGui::DragFloat3(
                    "Scale",
                    &transform.scale.x,
                    0.1f,
                    0.01f,
                    100.0f
                );
            }
        } else if (componentTypeID == ComponentTypeHelper<CameraComponent>::ID) {
            if (ImGui::CollapsingHeader("Camera", flags)) {
                auto &camera = componentManager->GetComponent<CameraComponent>(entity);

                const char *projectionTypes[] = {"Perspective", "Orthographic"};
                int currentType = camera.projection;
                if (ImGui::Combo("Projection", &currentType, projectionTypes, IM_ARRAYSIZE(projectionTypes))) {
                    camera.projection = static_cast<ProjectionType>(currentType);
                }

                ImGui::DragFloat("Field of View", &camera.fieldOfView, 0.1f, 1.0f, 179.0f);

                ImGui::DragFloat("Near Plane", &camera.nearPlane, 0.01f, 0.01f, camera.farPlane - 0.1f);

                ImGui::DragFloat("Far Plane", &camera.farPlane, 0.1f, camera.nearPlane + 0.1f, 10000.0f);

                ImGui::DragFloat("Aspect Ratio", &camera.aspectRatio, 0.01f, 0.0f, 2.0f);

                if (camera.projection == ORTHOGRAPHIC) {
                    ImGui::DragFloat("Ortho Scale", &camera.orthoScale, 0.1f, 0.1f, 1000.0f);
                }

                auto isActive = componentManager->HasComponent<ActiveCameraTagComponent>(entity);
                if (ImGui::Checkbox("Active Camera", &isActive)) {
                    if (isActive) {
                        constexpr ActiveCameraTagComponent tag{};
                        componentManager->AddComponent<ActiveCameraTagComponent>(entity, tag);
                    } else {
                        componentManager->RemoveComponent<ActiveCameraTagComponent>(entity);
                    }
                }
            }
        } else if (componentTypeID == ComponentTypeHelper<VelocityComponent>::ID) {
            if (ImGui::CollapsingHeader("Velocity", flags)) {
                auto &velocity = componentManager->GetComponent<VelocityComponent>(entity);

                ImGui::DragFloat3(
                    "Linear",
                    &velocity.linearVelocity.x,
                    0.1f
                );

                ImGui::DragFloat3(
                    "Angular",
                    &velocity.angularVelocity.x,
                    0.1f
                );
            }
        } else if (componentTypeID == ComponentTypeHelper<RenderableComponent>::ID) {
            if (ImGui::CollapsingHeader("Rendering", flags)) {
                auto &renderable = componentManager->GetComponent<RenderableComponent>(entity);

                ImGui::InputScalar(
                    "Mesh ID",
                    ImGuiDataType_U32,
                    &renderable.meshId,
                    nullptr,
                    nullptr,
                    "%u"
                );

                ImGui::InputScalar(
                    "Texture ID",
                    ImGuiDataType_U32,
                    &renderable.textureId,
                    nullptr,
                    nullptr,
                    "%u"
                );
            }
        } else if (componentTypeID == ComponentTypeHelper<LocalToWorldComponent>::ID) {
            if (showDebugInfo) {
                if (ImGui::CollapsingHeader("World Matrix (Debug)", flags)) {
                    auto &l2w = componentManager->GetComponent<LocalToWorldComponent>(entity);

                    ImGui::Text("Matrix isDirty: %s", l2w.isDirty ? "True" : "False");
                    ImGui::Separator();

                    for (int i = 0; i < 4; ++i) {
                        ImGui::Text(
                            "C%d: %.3f | %.3f | %.3f | %.3f", i,
                            l2w.localToWorldMatrix[i].x,
                            l2w.localToWorldMatrix[i].y,
                            l2w.localToWorldMatrix[i].z,
                            l2w.localToWorldMatrix[i].w
                        );
                    }
                }
            }
        } else if (componentTypeID == ComponentTypeHelper<ParentComponent>::ID) {
            if (ImGui::CollapsingHeader("Hierarchy Parent", flags)) {
                auto &parentComp = componentManager->GetComponent<ParentComponent>(entity);
                EntityID currentParentId = parentComp.parent;
                std::string currentParentName = (currentParentId == NULL_ENTITY)
                                                    ? "None"
                                                    : entityManager->GetEntityName(currentParentId);

                if (ImGui::BeginCombo("Parent", currentParentName.c_str())) {
                    bool is_none_selected = currentParentId == NULL_ENTITY;
                    if (ImGui::Selectable("None", &is_none_selected)) {
                        Utils::Entities::Hierarchy::SetParent(entity, NULL_ENTITY, componentManager);
                    }

                    // Option 2: List all other entities
                    // This is an expensive operation; a better solution is to cache entity lists.
                    // TODO: implement parent/child entity list caching to optimize this.
                    for (const auto &entityData: entityManager->GetAllEntities()) {
                        EntityID entityId = entityData.id;

                        if (entityId == entity || entityId == NULL_ENTITY) {
                            continue;
                        }

                        bool is_selected = (entityId == currentParentId);

                        if (ImGui::Selectable(entityData.name.c_str(), &is_selected)) {
                            Utils::Entities::Hierarchy::SetParent(entity, entityId, componentManager);
                        }
                    }
                    ImGui::EndCombo();
                }
            }
        } else if (componentTypeID == ComponentTypeHelper<ChildrenComponent>::ID) {
            if (ImGui::CollapsingHeader("Hierarchy Children", flags)) {
                auto &childrenComp = componentManager->GetComponent<ChildrenComponent>(entity);

                ImGui::Text("Children Count: %zu", childrenComp.children.size());
                ImGui::Separator();

                int childIndex = 0;
                for (EntityID childId: childrenComp.children) {
                    std::string childName = entityManager->GetEntityName(childId);
                    ImGui::Text("%d: %s", childIndex++, childName.c_str());
                    ImGui::SameLine();

                    ImGui::PushID(childId);
                    if (ImGui::SmallButton("Detach")) {
                        Utils::Entities::Hierarchy::SetParent(childId, NULL_ENTITY, componentManager);
                    }
                    ImGui::PopID();
                }
                ImGui::Separator();

                const std::string add_child_preview = "Add Existing Entity as Child";

                if (ImGui::BeginCombo("##AddChildCombo", add_child_preview.c_str())) {
                    for (const auto &entityData: entityManager->GetAllEntities()) {
                        EntityID entityId = entityData.id;

                        if (
                            entityId == entity
                            || entityId == NULL_ENTITY
                            || childrenComp.children.contains(entityId)
                        ) {
                            continue;
                        }

                        if (ImGui::Selectable(entityData.name.c_str())) {
                            Utils::Entities::Hierarchy::AddChild(
                                entity,
                                entityId,
                                componentManager
                            );
                        }
                    }
                    ImGui::EndCombo();
                }
            }
        } else if (componentTypeID == ComponentTypeHelper<PhysicsSettingsComponent>::ID) {
            if (ImGui::CollapsingHeader("Physics Settings", flags)) {
                auto &physicsSettings = componentManager->GetComponent<PhysicsSettingsComponent>(entity);

                ImGui::DragFloat(
                    "Gravity Acceleration",
                    &physicsSettings.gravityAcceleration,
                    0.1f,
                    0,
                    100.0f
                );

                static glm::vec3 tempGravityDir = physicsSettings.gravityDirection;

                ImGui::DragFloat3(
                    "Gravity Direction",
                    &tempGravityDir.x,
                    0.1f,
                    -1.0f,
                    1.0f
                );

                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    physicsSettings.gravityDirection = glm::normalize(tempGravityDir);
                }

                ImGui::DragInt(
                    "Solver Iterations",
                    &physicsSettings.solverIterations,
                    1,
                    1, 100
                );
            }
        } else if (componentTypeID == ComponentTypeHelper<PlayerControllerComponent>::ID) {
            if (ImGui::CollapsingHeader("Player Controller", flags)) {
                auto &playerController = componentManager->GetComponent<PlayerControllerComponent>(entity);

                ImGui::DragFloat(
                    "Movement Speed",
                    &playerController.movementSpeed,
                    0.1f,
                    0.1f,
                    100.0f
                );

                static glm::vec3 tempForwardDirection = playerController.forwardDirection;

                ImGui::DragFloat3(
                    "Forward Direction",
                    &tempForwardDirection.x,
                    0.1f,
                    -1.0f,
                    1.0f
                );

                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    playerController.forwardDirection = glm::normalize(tempForwardDirection);
                }
            }
        }
    }

    if (!isInternal) {
        ImGui::Separator();

        const auto preview_value = "Add Component";

        if (ImGui::BeginCombo("##AddComponent", preview_value)) {
            for (const auto &componentId: componentManager->GetRegisteredComponents()) {
                if (componentId == ComponentTypeHelper<InternalTagComponent>::ID) {
                    continue; // Skip internal components
                }

                const bool isAlreadyPresent = componentManager->HasComponent(componentId, entity);
                const std::string componentName = componentManager->GetComponentName(componentId);

                ImGuiSelectableFlags flags = 0;
                if (isAlreadyPresent) {
                    flags |= ImGuiSelectableFlags_Disabled;
                }

                if (ImGui::Selectable(componentName.c_str(), false, flags)) {
                    componentManager->AddDefaultComponent(componentId, entity);
                    if (componentId == ComponentTypeHelper<LocalTransformComponent>::ID) {
                        componentManager->AddComponent<LocalToWorldComponent>(entity, {});
                    }
                }
            }

            ImGui::EndCombo();
        }
    }
}

void Editor::UI::Inspector::Draw(const char *title, VeeEditor *editor) {
    ImGui::Begin(title);

    const auto selectedEntity = editor->GetSelectedEntity();
    const auto scene = editor->GetScene();
    if (editor->GetSelectedEntity() == NULL_ENTITY) {
        DrawSceneInspector(editor, scene);
    } else {
        DrawEntityInspector(editor, scene, selectedEntity);
    }

    ImGui::End();
}
