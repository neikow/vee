#include "editor.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"
#include "renderer/vulkan/vulkan_renderer_with_ui.h"
bool show_demo_window = true;

// cpp
void Editor::DrawSceneHierarchy() {
    ImGui::Begin("Scene Hierarchy");

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
        for (const auto &sceneData: m_SceneManager->ListScenes()) {
            const bool isSelected = (m_Engine->GetScene()->GetName() == sceneData.name);
            if (ImGui::Selectable(sceneData.name.c_str(), isSelected)) {
                m_SceneManager->LoadScene(sceneData.path);
            }
            if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine(0.0f, spacing);
    if (ImGui::Button("+", ImVec2(buttonWidth, 0.0f))) {
        // handle add scene
    }

    ImGui::End();
}


void Editor::DrawInspector() {
    {
        ImGui::Begin("Component Inspector");
        ImGui::Text("Hello from the editor!");
        ImGui::End();
    }
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
            m_Engine->Reset();
        }

        ImGui::Separator();

        ImGui::Image(textureID, viewportPanelSize);
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void Editor::DrawAssetManager() {
    {
        ImGui::Begin("Assets Manager");
        ImGui::Text("Hello from the editor!");
        ImGui::End();
    }
}

void Editor::Run(const int width, const int height) {
    m_Engine->Initialize(width, height, "Editor", VK_MAKE_VERSION(1, 0, 0));

    auto lastTime = std::chrono::high_resolution_clock::now();

    while (!m_Engine->GetRenderer()->ShouldClose()) {
        glfwPollEvents();

        const auto currentTime = std::chrono::high_resolution_clock::now();
        const float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        m_Engine->Update(deltaTime);

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

    m_Engine->Shutdown();
}
