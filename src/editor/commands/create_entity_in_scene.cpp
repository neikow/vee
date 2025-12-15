#include "create_entity_in_scene.h"
#include "../editor.h"

Editor::Command::CreateEntityInScene::CreateEntityInScene(VeeEditor *editor) : m_Editor(editor) {
}

void Editor::Command::CreateEntityInScene::Execute() {
    const auto engine = m_Editor->GetEngine();
    const auto createdEntityID = engine->GetScene()->CreateEntity("Unnamed Entity");
    m_Editor->SelectEntity(createdEntityID);
}
