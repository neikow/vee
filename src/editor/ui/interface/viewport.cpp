#include "viewport.h"

#include "imgui_internal.h"
#include "../../commands/reload_current_scene_from_file.h"
#include "../../renderer/vulkan/vulkan_renderer_with_ui.h"

void Editor::UI::Viewport::Draw(const char *title, VeeEditor *editor) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    const auto engine = editor->GetEngine();
    const auto renderer = std::reinterpret_pointer_cast<Vulkan::RendererWithUi>(
        engine->GetRenderer()
    );

    ImGuiWindowClass window_class;
    window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
    ImGui::SetNextWindowClass(&window_class);

    if (
        ImGui::Begin(
            title,
            nullptr,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoTitleBar
        )
    ) {
        const ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

        auto &state = editor->GetEditorState();

        state.isViewportFocused = ImGui::IsWindowFocused();
        state.isViewportHovered = ImGui::IsWindowHovered();
        state.viewportSize = {
            static_cast<uint32_t>(viewportPanelSize.x),
            static_cast<uint32_t>(viewportPanelSize.y)
        };
        renderer->UpdateViewportSize(
            static_cast<uint32_t>(viewportPanelSize.x),
            static_cast<uint32_t>(viewportPanelSize.y)
        );

        const VkDescriptorSet &textureID = renderer->GetViewportDescriptorSet();

        const char *label = engine->Paused() ? "Play >" : "Pause ||";
        if (ImGui::Button(label, ImVec2(80, 0))) {
            if (engine->Paused()) {
                engine->Resume();
            } else {
                engine->Pause();
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Reset")) {
            Command::ReloadCurrentSceneFromFile(editor).Execute();
        }

        ImGui::Separator();

        ImGui::Image(textureID, viewportPanelSize);

        if (state.isViewportHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            const double mouseX = InputSystem::GetMouseX(), mouseY = InputSystem::GetMouseY();
            const ImVec2 imageMin = ImGui::GetItemRectMin();
            const auto normX = (mouseX - imageMin.x) / viewportPanelSize.x,
                    normY = (mouseY - imageMin.y) / viewportPanelSize.y;
            editor->RequestEntitySelectionWithinViewport(normX, normY);
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
}
