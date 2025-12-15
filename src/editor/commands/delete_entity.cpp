#include "delete_entity.h"

#include "../editor.h"
#include "../../engine/entities/types.h"

Editor::Command::DeleteEntity::DeleteEntity(VeeEditor *editor) : m_Editor(editor) {
}

void Editor::Command::DeleteEntity::Execute() {
    const auto engine = m_Editor->GetEngine();
    const auto selectedEntity = m_Editor->GetSelectedEntity();
    if (selectedEntity == NULL_ENTITY) return;

    const auto scene = engine->GetScene();

    if (scene->DestroyEntity(selectedEntity)) {
        m_Editor->SelectEntity(NULL_ENTITY);
    }
}

