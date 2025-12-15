#include "save_scene_as_modal.h"

#include "../../../../engine/utils/strings.h"
#include "../../../scenes/scene_manager.h"

Editor::UI::Modals::SaveSceneAsModal::SaveSceneAsModal(SceneManager *sceneManager) : m_SceneManager(sceneManager) {
}

void Editor::UI::Modals::SaveSceneAsModal::Draw() {
    if (ImGui::BeginPopupModal(SAVE_SCENE_AS, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
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
