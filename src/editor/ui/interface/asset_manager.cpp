#include "asset_manager.h"

#include "imgui_internal.h"

void Editor::UI::AssetManager::Draw(const char *title, VeeEditor *) {
    ImGui::Begin(title);

    ImGui::Text("Hello from the asset manager!");

    ImGui::End();
}
