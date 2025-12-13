#include "editor.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "../engine/entities/components_system/components/renderable_component.h"
#include "../engine/entities/components_system/components/camera_component.h"
#include "../engine/entities/components_system/components/transform_component.h"
#include "../engine/entities/components_system/components/velocity_component.h"
#include "../engine/entities/components_system/tags/active_camera_tag_component.h"
#include "../engine/entities/components_system/tags/editor_camera_tag_component.h"
#include "../engine/entities/components_system/tags/internal_tag_component.h"
#include "../engine/io/input_system.h"
#include "../engine/utils/strings.h"
#include "renderer/vulkan/vulkan_renderer_with_ui.h"
#include "systems/editor_camera_system.h"
#include "ui/editor_console.h"


void Editor::DrawCurrentSceneHierarchy() {
    const auto scene = m_Engine->GetScene();
    for (const auto &entity: scene->GetEntityManager()->GetAllEntities()) {
        ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow;

        const bool isInternal = m_Engine->GetScene()->GetComponentManager()->HasComponent<
            InternalTagComponent>(entity.id);

        if (!m_EditorSettings.displayInternalEntities && isInternal) {
            continue;
        }

        if (entity.id == m_SelectedEntity) {
            nodeFlags |= ImGuiTreeNodeFlags_Selected;
        }

        const bool nodeOpen = ImGui::TreeNodeEx(
            (void *) (uintptr_t) entity.id,
            nodeFlags,
            isInternal ? "%s (Intenal)" : "%s",
            Utils::Strings::TruncateString(entity.name, 20).c_str()
        );

        if (ImGui::IsItemClicked()) {
            SelectEntity(entity.id);
        }

        if (nodeOpen) {
            ImGui::TreePop();
        }
    }
}

void Editor::NewEmptyScene() {
    m_SelectedEntity = NULL_ENTITY;
    m_SceneManager->NewEmptyScene();
    CreateEditorInternalEntities();
}

void Editor::DrawScenePicker() {
    const float availWidth = ImGui::GetContentRegionAvail().x;
    const ImGuiStyle &style = ImGui::GetStyle();
    const char *currentName = m_Engine->GetScene()->GetName().c_str();

    const float plusTextW = ImGui::CalcTextSize("+").x;
    float buttonWidth = plusTextW + style.FramePadding.x * 2.0f;
    if (buttonWidth < 26.0f) buttonWidth = 26.0f;
    const float spacing = style.ItemInnerSpacing.x;

    float comboWidth = availWidth - buttonWidth - spacing;
    if (comboWidth < 0.0f) comboWidth = 0.0f;
    ImGui::SetNextItemWidth(comboWidth);

    if (ImGui::BeginCombo("##Scenes", currentName)) {
        for (const auto &sceneData: SceneManager::ListScenes()) {
            const bool isSelected = (m_Engine->GetScene()->GetName() == sceneData.name);
            if (ImGui::Selectable(sceneData.name.c_str(), isSelected)) {
                LoadScene(sceneData.path);
            }
            if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine(0.0f, spacing);
    if (ImGui::Button("+", ImVec2(buttonWidth, 0.0f))) {
        NewEmptyScene();
    }
}

void Editor::DrawSceneHierarchy() {
    ImGui::Begin("Scene Hierarchy");

    DrawScenePicker();
    ImGui::Separator();
    DrawCurrentSceneHierarchy();

    ImGui::End();
}


void Editor::DrawEntityInspector() const {
    const auto componentManager = m_Engine->GetScene()->GetComponentManager();
    const auto entityComponents = componentManager->GetEntityComponents(m_SelectedEntity);
    const auto isInternal = componentManager->HasComponent<InternalTagComponent>(m_SelectedEntity);

    const std::string entityNameString = m_Engine->GetScene()->GetEntityManager()->GetEntityName(m_SelectedEntity);
    char entityName[256];
    std::strncpy(entityName, entityNameString.c_str(), sizeof(entityName));
    ImGui::InputText(
        "Name",
        entityName,
        IM_ARRAYSIZE(entityName)
    );
    if (const auto newName = std::string(entityName); newName != entityNameString) {
        m_Engine->GetScene()->GetEntityManager()->RenameEntity(
            m_SelectedEntity,
            newName
        );
    }

    ImGui::Separator();

    for (const auto &componentTypeID: entityComponents) {
        constexpr ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;

        if (componentTypeID == ComponentTypeHelper<TransformComponent>::ID) {
            if (ImGui::CollapsingHeader("Transform", flags)) {
                auto &transform = componentManager->GetComponent<TransformComponent>(m_SelectedEntity);

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
            } // Contents block ends here
        } else if (componentTypeID == ComponentTypeHelper<CameraComponent>::ID) {
            if (ImGui::CollapsingHeader("Camera", flags)) {
                auto &camera = componentManager->GetComponent<CameraComponent>(m_SelectedEntity);

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

                auto isActive = componentManager->HasComponent<ActiveCameraTagComponent>(m_SelectedEntity);
                if (ImGui::Checkbox("Active Camera", &isActive)) {
                    if (isActive) {
                        constexpr ActiveCameraTagComponent tag{};
                        componentManager->AddComponent<ActiveCameraTagComponent>(m_SelectedEntity, tag);
                    } else {
                        componentManager->RemoveComponent<ActiveCameraTagComponent>(m_SelectedEntity);
                    }
                }
            }
        } else if (componentTypeID == ComponentTypeHelper<VelocityComponent>::ID) {
            if (ImGui::CollapsingHeader("Velocity", flags)) {
                auto &velocity = componentManager->GetComponent<VelocityComponent>(m_SelectedEntity);

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
                auto &renderable = componentManager->GetComponent<RenderableComponent>(m_SelectedEntity);

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

                const bool isAlreadyPresent = componentManager->HasComponent(componentId, m_SelectedEntity);
                const std::string componentName = componentManager->GetComponentName(componentId);

                ImGuiSelectableFlags flags = 0;
                if (isAlreadyPresent) {
                    flags |= ImGuiSelectableFlags_Disabled;
                }

                if (ImGui::Selectable(componentName.c_str(), false, flags)) {
                    componentManager->AddDefaultComponent(componentId, m_SelectedEntity);
                }
            }

            ImGui::EndCombo();
        }
    }
}

void Editor::DrawSceneInspector() {
    ImGui::Text("DEBUG: Current camera entityID : %d", m_Engine->GetActiveCameraEntityId());
}

void Editor::DrawInspector() {
    ImGui::Begin("Inspector");

    if (m_SelectedEntity == NULL_ENTITY) {
        DrawSceneInspector();
        ImGui::End();
        return;
    }

    DrawEntityInspector();

    ImGui::End();
}

void Editor::ReloadCurrentSceneFromFile() {
    m_Engine->Pause();
    LoadScene(m_Engine->GetScene()->GetPath());
}

void Editor::HandleEntitySelectionWithinViewport(const double normX, const double normY) {
    const auto renderer = std::static_pointer_cast<Vulkan::RendererWithUi>(m_Engine->GetRenderer());
    m_SelectedEntity = renderer->GetEntityIDAt(normX, normY);
}

void Editor::DrawViewport() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    const auto renderer = std::reinterpret_pointer_cast<Vulkan::RendererWithUi>(m_Engine->GetRenderer());

    ImGuiWindowClass window_class;
    window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
    ImGui::SetNextWindowClass(&window_class);

    if (
        ImGui::Begin(
            "Viewport",
            nullptr,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoTitleBar
        )
    ) {
        const ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

        m_State.isViewportFocused = ImGui::IsWindowFocused();
        m_State.isViewportHovered = ImGui::IsWindowHovered();

        m_State.viewportSize = {
            static_cast<uint32_t>(viewportPanelSize.x),
            static_cast<uint32_t>(viewportPanelSize.y)
        };
        renderer->UpdateViewportSize(
            static_cast<uint32_t>(viewportPanelSize.x),
            static_cast<uint32_t>(viewportPanelSize.y)
        );

        const VkDescriptorSet textureID = renderer->GetViewportDescriptorSet();

        const char *label = m_Engine->Paused() ? "Play >" : "Pause ||";
        if (ImGui::Button(label, ImVec2(80, 0))) {
            if (m_Engine->Paused()) {
                m_Engine->Resume();
            } else {
                m_Engine->Pause();
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Reset")) {
            ReloadCurrentSceneFromFile();
        }

        ImGui::Separator();

        ImGui::Image(textureID, viewportPanelSize);

        if (m_State.isViewportHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            const double mouseX = InputSystem::GetMouseX(), mouseY = InputSystem::GetMouseY();
            const ImVec2 imageMin = ImGui::GetItemRectMin();
            const auto normX = (mouseX - imageMin.x) / viewportPanelSize.x,
                    normY = (mouseY - imageMin.y) / viewportPanelSize.y;
            HandleEntitySelectionWithinViewport(normX, normY);
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void Editor::DrawAssetManager() const {
    {
        ImGui::Begin("Assets Manager");
        ImGui::Text("Hello from the editor!");
        ImGui::End();
    }
}

void Editor::DrawModals() {
    if (m_State.shouldSaveSceneAsModalOpen) {
        ImGui::OpenPopup("Provide a Scene name", ImGuiWindowFlags_AlwaysAutoResize);
        m_State.shouldSaveSceneAsModalOpen = false;
    }

    if (ImGui::BeginPopupModal("Provide a Scene name", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::SetItemDefaultFocus();

        ImGui::Text("Scene Name:");
        static char sceneNameBuffer[256] = "";
        ImGui::InputText("##SceneNameInput", sceneNameBuffer, IM_ARRAYSIZE(sceneNameBuffer));

        const auto sceneFileName = Utils::Strings::ToLower(
            Utils::Strings::ReplaceAll(
                Utils::Strings::TrimWhitespace(
                    sceneNameBuffer
                ),
                " ",
                "_"
            )
        );

        ImGui::Text(
            "%s.scene",
            sceneFileName.c_str()
        );
        ImGui::Separator();


        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        ImGui::BeginDisabled(sceneFileName.empty());
        if (ImGui::Button("Save", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();

            m_SceneManager->SaveScene(
                "../.editor_data/scenes/" + sceneFileName + ".scene",
                sceneNameBuffer
            );
        }
        ImGui::EndDisabled();
        ImGui::EndPopup();
    }
}

void Editor::DrawUI() {
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(
        ImGui::GetID("MyDockSpace"),
        ImGui::GetMainViewport()
    );

    DrawSceneHierarchy();
    DrawInspector();
    DrawAssetManager();
    EditorConsole::Draw("Console", nullptr);
    DrawViewport();

    DrawModals();

    ImGui::Render();
}

void Editor::Run(const int width, const int height) {
    m_Engine->Initialize(width, height, "Editor", VK_MAKE_VERSION(1, 0, 0));

    auto lastTime = std::chrono::high_resolution_clock::now();

    m_ShortcutManager->RegisterShortcut(
        SHORTCUT_RELOAD,
        {KEY_LEFT_CONTROL, KEY_R},
        [this] {
            ReloadCurrentSceneFromFile();
        }
    );
    m_ShortcutManager->RegisterShortcut(
        SHORTCUT_SAVE,
        {KEY_SUPER, KEY_S},
        [this] {
            const auto scene = m_Engine->GetScene();

            const auto scenePath = scene->GetPath();

            if (scenePath.empty()) {
                m_State.shouldSaveSceneAsModalOpen = true;
                return;
            }

            m_SceneManager->SaveScene(
                scenePath,
                scene->GetName()
            );
        }
    );
    m_ShortcutManager->RegisterShortcut(
        SHORTCUT_ADD_ENTITY_TO_SCENE,
        {KEY_LEFT_CONTROL, KEY_SPACE},
        [this] {
            m_SelectedEntity = m_Engine->GetScene()->CreateEntity("Unnamed Entity");
        }
    );
    m_ShortcutManager->RegisterShortcut(
        SHORTCUT_DELETE,
        {KEY_LEFT_CONTROL, KEY_DELETE},
        [this] {
            if (m_SelectedEntity == NULL_ENTITY) return;

            const auto scene = m_Engine->GetScene();

            if (scene->DestroyEntity(m_SelectedEntity)) {
                m_SelectedEntity = NULL_ENTITY;
            }
        }
    );

    while (!m_Engine->GetRenderer()->ShouldClose()) {
        glfwPollEvents();

        const auto currentTime = std::chrono::high_resolution_clock::now();
        const float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        m_Engine->UpdateSystems(deltaTime);

        m_Engine->PrepareForRendering();

        DrawUI();

        m_Engine->GetRenderer()->Draw();

        m_ShortcutManager->HandleShortcuts();

        InputSystem::UpdateEndOfFrame();
    }

    m_Engine->Shutdown();
}

void Editor::CreateEditorCamera(const std::shared_ptr<Scene> &scene) {
    const auto editorCamera = scene->CreateEntity("Editor Camera");
    const auto componentManager = scene->GetComponentManager();
    const auto systemManager = scene->GetSystemManager();

    componentManager->AddComponent<InternalTagComponent>(
        editorCamera,
        InternalTagComponent{}
    );

    componentManager->AddComponent<TransformComponent>(
        editorCamera,
        TransformComponent{
            .position = glm::vec3(0.0f, 0.15f, 3.0f),
            .rotation = glm::quat(0.784934, 0.392467, 0.214406, 0.428811),
            .scale = glm::vec3(1.0f)
        }
    );
    componentManager->AddComponent<CameraComponent>(
        editorCamera,
        CameraComponent{
            .projection = PERSPECTIVE,
            .fieldOfView = 90.0f,
            .aspectRatio = 0.0f,
            .nearPlane = 0.1f,
            .farPlane = 1000.0f
        }
    );
    componentManager->AddComponent<EditorCameraTagComponent>(
        editorCamera,
        EditorCameraTagComponent{}
    );

    m_EditorCameraSignature = m_Engine->GetScene()->GetEntityManager()->GetSignature(editorCamera);

    m_Engine->SetActiveCameraEntityId(editorCamera);
}

void Editor::CreateEditorInternalEntities() {
    CreateEditorCamera(m_Engine->GetScene());
}
