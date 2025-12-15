#include "reload_current_scene_from_file.h"
#include "../editor.h"

Editor::Command::ReloadCurrentSceneFromFile::ReloadCurrentSceneFromFile(VeeEditor *editor) : m_Editor(editor) {
}

void Editor::Command::ReloadCurrentSceneFromFile::Execute() {
    const auto engine = m_Editor->GetEngine();
    engine->Pause();
    m_Editor->LoadScene(
        engine->GetScene()->GetPath()
    );
}
