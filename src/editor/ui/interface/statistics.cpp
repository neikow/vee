#include "statistics.h"

void Editor::UI::Statistics::DrawRendererStatistics(const std::shared_ptr<AbstractRenderer> &renderer) {
    const auto memoryUsage = renderer->GetMemoryUsage();

    ImGui::Text("Renderer Statistics:");
    ImGui::Separator();
    ImGui::Text("Memory Usage:");
    ImGui::BulletText("Available Memory: %.2f MB", memoryUsage.availableMemoryMB);
    ImGui::BulletText("Used Memory: %.2f MB", memoryUsage.usedMemoryMB);
}

void Editor::UI::Statistics::Draw(const char *title, const VeeEditor *editor) {
    ImGui::Begin(title);

    DrawRendererStatistics(editor->GetEngine()->GetRenderer());

    ImGui::End();
}
