#include "save_scene.h"
#include "../editor.h"

Editor::Command::SaveScene::SaveScene(VeeEditor *editor) : m_Editor(editor) {
}

void Editor::Command::SaveScene::Execute() {
    const auto scene = m_Editor->GetEngine()->GetScene();

    const auto scenePath = scene->GetPath();

    if (scenePath.empty()) {
        m_Editor->GetUIManager()->OpenModal(UI::Modals::SAVE_SCENE_AS);
        return;
    }

    m_Editor->GetSceneManager()->SaveScene(
        scenePath,
        scene->GetName()
    );
}
