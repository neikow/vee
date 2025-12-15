#include "editor_ui_manager.h"

#include "imgui_internal.h"

void Editor::UIManager::OpenModal(const char *modalName) {
    m_ModalsVisibility[modalName] = true;
    ImGui::OpenPopup(modalName);
}

bool Editor::UIManager::ShouldDrawModal(const char *modalName) const {
    const auto it = m_ModalsVisibility.find(modalName);
    return it != m_ModalsVisibility.end() && it->second;
}

void Editor::UIManager::CloseModal(const char *modalName) {
    m_ModalsVisibility[modalName] = false;
    ImGui::CloseCurrentPopup();
}
