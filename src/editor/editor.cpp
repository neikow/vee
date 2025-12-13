#include "editor.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "../engine/entities/components_system/components/renderable_component.h"
#include "../engine/entities/components_system/components/camera_component.h"
#include "../engine/entities/components_system/components/children_component.h"
#include "../engine/entities/components_system/components/local_to_world_component.h"
#include "../engine/entities/components_system/components/local_transform_component.h"
#include "../engine/entities/components_system/components/velocity_component.h"
#include "../engine/entities/components_system/tags/active_camera_tag_component.h"
#include "../engine/entities/components_system/tags/editor_camera_tag_component.h"
#include "../engine/entities/components_system/tags/internal_tag_component.h"
#include "../engine/io/input_system.h"
#include "../engine/utils/strings.h"
#include "../engine/utils/entities/hierarchy.h"
#include "renderer/vulkan/vulkan_renderer_with_ui.h"
#include "systems/editor_camera_system.h"
#include "ui/asset_manager.h"
#include "ui/editor_console.h"
#include "ui/inspector.h"
#include "ui/scene_hierarchy.h"
#include "ui/viewport.h"

void VeeEditor::NewEmptyScene() {
    m_SelectedEntity = NULL_ENTITY;
    m_SceneManager->NewEmptyScene();
    CreateEditorInternalEntities();
}

void VeeEditor::ReloadCurrentSceneFromFile() {
    m_Engine->Pause();
    LoadScene(m_Engine->GetScene()->GetPath());
}

void VeeEditor::HandleEntitySelectionWithinViewport(const double normX, const double normY) {
    const auto renderer = std::static_pointer_cast<Vulkan::RendererWithUi>(m_Engine->GetRenderer());
    m_SelectedEntity = renderer->GetEntityIDAt(normX, normY);
}

void VeeEditor::DrawModals() {
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

void VeeEditor::DrawUI() {
    using namespace Editor::UI;

    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(
        ImGui::GetID("MyDockSpace"),
        ImGui::GetMainViewport()
    );

    SceneHierarchy::Draw("Scene Hierarchy", this);
    Inspector::Draw("Inspector", this);
    AssetManager::Draw("Asset Manager", this);
    Console::Draw("Console", this);
    Viewport::Draw("Viewport", this);

    DrawModals();

    ImGui::Render();
}

void VeeEditor::Run(const int width, const int height) {
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

void VeeEditor::CreateEditorCamera(const std::shared_ptr<Scene> &scene) {
    const auto editorCamera = scene->CreateEntity("Editor Camera");
    const auto componentManager = scene->GetComponentManager();
    const auto systemManager = scene->GetSystemManager();

    componentManager->AddComponent<InternalTagComponent>(
        editorCamera,
        InternalTagComponent{}
    );

    componentManager->AddComponent<LocalTransformComponent>(
        editorCamera,
        LocalTransformComponent{
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

void VeeEditor::CreateEditorInternalEntities() {
    CreateEditorCamera(m_Engine->GetScene());
}
