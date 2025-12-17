#include "editor.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "../engine/entities/components_system/components/renderable_component.h"
#include "../engine/entities/components_system/components/camera_component.h"
#include "../engine/entities/components_system/components/children_component.h"
#include "../engine/entities/components_system/components/local_transform_component.h"
#include "../engine/entities/components_system/tags/editor_camera_tag_component.h"
#include "../engine/entities/components_system/tags/internal_tag_component.h"
#include "../engine/io/input_system.h"
#include "../engine/utils/entities/hierarchy.h"
#include "commands/create_entity_in_scene.h"
#include "commands/delete_entity.h"
#include "commands/reload_current_scene_from_file.h"
#include "commands/save_scene.h"
#include "renderer/vulkan/vulkan_renderer_with_ui.h"
#include "systems/editor_camera_system.h"
#include "ui/interface/asset_manager.h"
#include "ui/interface/editor_console.h"
#include "ui/interface/inspector.h"
#include "ui/interface/scene_hierarchy.h"
#include "ui/interface/statistics.h"
#include "ui/interface/viewport.h"
#include "ui/interface/modals/save_scene_as_modal.h"
#include "ui/interface/modals/types.h"

void VeeEditor::NewEmptyScene() {
    m_State.selectedEntity = NULL_ENTITY;
    m_SceneManager->NewEmptyScene();
    CreateEditorInternalEntities();
}

EditorSettings VeeEditor::GetEditorSettings() const { return m_EditorSettings; }

EditorSettings &VeeEditor::GetEditorSettings() { return m_EditorSettings; }

EditorState VeeEditor::GetEditorState() const { return m_State; }

EditorState &VeeEditor::GetEditorState() { return m_State; }

void VeeEditor::SelectEntity(const EntityID entityID) { m_State.selectedEntity = entityID; }

void VeeEditor::RequestEntitySelectionWithinViewport(const double normX, const double normY) {
    const auto renderer = std::static_pointer_cast<Vulkan::RendererWithUi>(m_Engine->GetRenderer());
    renderer->RequestEntityIDAt(normX, normY);
    m_State.isWaitingForEntitySelection = true;
}

void VeeEditor::DrawModals() const {
    using namespace Editor::UI::Modals;

    SaveSceneAsModal(m_SceneManager.get()).Draw();
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
    Statistics::Draw("Statistics", this);

    DrawModals();
}

VeeEditor::VeeEditor(const std::shared_ptr<Engine> &engine) : m_Engine(engine) {
    m_UIManager = std::make_shared<Editor::UIManager>();
    m_SceneManager = std::make_shared<SceneManager>(m_Engine);
    m_ShortcutManager = std::make_unique<ShortcutManager>();
}

void VeeEditor::Run(const int width, const int height) {
    m_Engine->Initialize(width, height, "Editor", VK_MAKE_VERSION(1, 0, 0));

    auto lastTime = std::chrono::high_resolution_clock::now();

    const auto renderer = std::static_pointer_cast<Vulkan::RendererWithUi>(
        m_Engine->GetRenderer()
    );

    while (!renderer->ShouldClose()) {
        glfwPollEvents();
        if (m_State.isWaitingForEntitySelection && !renderer->IsPickingRequestPending()) {
            m_State.selectedEntity = renderer->GetLastPickedID();
            m_State.isWaitingForEntitySelection = false;
        }

        const auto currentTime = std::chrono::high_resolution_clock::now();
        const float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        m_Engine->UpdateSystems(deltaTime);

        m_Engine->PrepareForRendering();

        DrawUI();

        m_ShortcutManager->HandleShortcuts();

        ImGui::Render();

        m_Engine->GetRenderer()->Draw();

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

void VeeEditor::LoadScene(const std::string &path) {
    m_Engine->Pause();
    m_SceneManager->LoadScene(path);
    CreateEditorInternalEntities();
}

EntityID VeeEditor::GetSelectedEntity() const { return m_State.selectedEntity; }

std::shared_ptr<Scene> VeeEditor::GetScene() const { return m_Engine->GetScene(); }

std::shared_ptr<Engine> VeeEditor::GetEngine() const { return m_Engine; }

std::shared_ptr<Editor::UIManager> VeeEditor::GetUIManager() const { return m_UIManager; }

std::shared_ptr<SceneManager> VeeEditor::GetSceneManager() { return m_SceneManager; }

void VeeEditor::RegisterShortcuts() {
    m_ShortcutManager->RegisterShortcut(
        SHORTCUT_RELOAD,
        {GLFW_KEY_LEFT_CONTROL, GLFW_KEY_R},
        [this] {
            Editor::Command::ReloadCurrentSceneFromFile(this).Execute();
        }
    );
    m_ShortcutManager->RegisterShortcut(
        SHORTCUT_SAVE,
        {GLFW_KEY_LEFT_SUPER, GLFW_KEY_S},
        [this] {
            Editor::Command::SaveScene(this).Execute();
        }
    );
    m_ShortcutManager->RegisterShortcut(
        SHORTCUT_ADD_ENTITY_TO_SCENE,
        {GLFW_KEY_LEFT_CONTROL, GLFW_KEY_SPACE},
        [this] {
            Editor::Command::CreateEntityInScene(this).Execute();
        }
    );
    m_ShortcutManager->RegisterShortcut(
        SHORTCUT_DELETE,
        {GLFW_KEY_LEFT_CONTROL, GLFW_KEY_DELETE},
        [this] {
            Editor::Command::DeleteEntity(this).Execute();
        }
    );
}

void VeeEditor::RegisterInternalSystems() {
    m_Engine->RegisterSystems(
        [this](
    const auto &,
    const auto &systemManager,
    const auto &componentManager
) {
            componentManager->template RegisterComponent<EditorCameraTagComponent>(
                VEE_EDITOR_CAMERA_TAG_COMPONENT_NAME
            );

            Signature editorCameraSignature;
            editorCameraSignature.set(ComponentTypeHelper<CameraComponent>::ID);
            editorCameraSignature.set(ComponentTypeHelper<LocalTransformComponent>::ID);
            editorCameraSignature.set(ComponentTypeHelper<EditorCameraTagComponent>::ID);
            systemManager->template RegisterSystem<EditorCameraSystem>(
                std::make_shared<EditorCameraSystem>(
                    m_Engine->GetRenderer(),
                    componentManager,
                    this
                )
            );
            systemManager->template SetSignature<EditorCameraSystem>(editorCameraSignature);
        }
    );
}

void VeeEditor::Initialize() {
    RegisterInternalSystems();
    RegisterShortcuts();
}
