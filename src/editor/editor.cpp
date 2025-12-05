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
#include "renderer/vulkan/vulkan_renderer_with_ui.h"
#include "systems/editor_camera_system.h"


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
            isInternal ? "Entity %d (Intenal)" : "Entity %d",
            entity.id
        );

        if (ImGui::IsItemClicked()) {
            SelectEntity(entity.id);
        }

        if (nodeOpen) {
            ImGui::TreePop();
        }
    }
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
        m_SceneManager->NewEmptyScene();
    }
}

void Editor::DrawSceneHierarchy() {
    ImGui::Begin("Scene Hierarchy");

    DrawScenePicker();
    ImGui::Separator();
    DrawCurrentSceneHierarchy();

    ImGui::End();
}


void Editor::DrawInspector() {
    const auto componentManager = m_Engine->GetScene()->GetComponentManager();
    ImGui::Begin("Component Inspector");

    if (m_SelectedEntity == NULL_ENTITY) {
        ImGui::Text("No entity selected.");
        ImGui::End();
        return;
    }

    const auto entityComponents = componentManager->GetEntityComponents(m_SelectedEntity);
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

void Editor::DrawUI() {
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(
        ImGui::GetID("MyDockSpace"),
        ImGui::GetMainViewport()
    );

    DrawSceneHierarchy();
    DrawInspector();
    DrawAssetManager();
    DrawViewport();

    ImGui::Render();
    m_Engine->GetRenderer()->SubmitUIDrawData(ImGui::GetDrawData());
}

void Editor::Run(const int width, const int height) {
    m_Engine->Initialize(width, height, "Editor", VK_MAKE_VERSION(1, 0, 0));

    auto lastTime = std::chrono::high_resolution_clock::now();

    while (!m_Engine->GetRenderer()->ShouldClose()) {
        glfwPollEvents();

        const auto currentTime = std::chrono::high_resolution_clock::now();
        const float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        m_Engine->UpdateSystems(deltaTime);

        m_Engine->PrepareForRendering();

        DrawUI();

        m_Engine->GetRenderer()->Draw();

        InputSystem::UpdateEndOfFrame();
    }

    m_Engine->Shutdown();
}

void Editor::CreateEditorCamera(const std::shared_ptr<Scene> &scene) const {
    scene->GetComponentManager()->RegisterComponent<EditorCameraTagComponent>();

    const auto editorCamera = scene->CreateEntity();
    const auto componentManager = scene->GetComponentManager();
    const auto systemManager = scene->GetSystemManager();

    Signature editorCameraSignature;
    editorCameraSignature.set(ComponentTypeHelper<CameraComponent>::ID);
    editorCameraSignature.set(ComponentTypeHelper<TransformComponent>::ID);
    editorCameraSignature.set(ComponentTypeHelper<EditorCameraTagComponent>::ID);
    systemManager->RegisterSystem<EditorCameraSystem>(
        std::make_shared<EditorCameraSystem>(
            m_Engine->GetRenderer(),
            componentManager,
            this
        )
    );
    systemManager->SetSignature<EditorCameraSystem>(editorCameraSignature);

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
            .aspectRatio = 16.0f / 9.0f,
            .nearPlane = 0.1f,
            .farPlane = 1000.0f
        }
    );
    componentManager->AddComponent<EditorCameraTagComponent>(
        editorCamera,
        EditorCameraTagComponent{}
    );

    m_Engine->SetActiveCameraEntityId(editorCamera);
}

void Editor::CreateEditorInternalEntities() const {
    const auto scene = m_Engine->GetScene();

    CreateEditorCamera(scene);
}
