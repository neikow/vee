#include "scene_hierarchy.h"

void Editor::UI::SceneHierarchy::DrawHierarchy(VeeEditor *editor, const std::shared_ptr<Scene> &scene) {
    const auto entityManager = scene->GetEntityManager();
    const auto componentManager = scene->GetComponentManager();

    // 1. Identify all root entities to begin the draw.
    // We iterate over all entities and filter for those without a ParentComponent
    // or whose ParentComponent points to NULL_ENTITY.
    for (const auto &entityData: entityManager->GetAllEntities()) {
        const EntityID entityID = entityData.id;

        // Check for ParentComponent presence and value
        bool isRoot = true;
        if (componentManager->HasComponent<ParentComponent>(entityID)) {
            const auto &parentComp = componentManager->GetComponent<ParentComponent>(entityID);
            if (parentComp.parent != NULL_ENTITY) {
                isRoot = false;
            }
        }

        // Skip internal entities unless debug info is enabled
        const bool isInternal = componentManager->HasComponent<InternalTagComponent>(entityID);
        if (!editor->GetEditorSettings().displayDebugInfo && isInternal) {
            continue;
        }

        if (isRoot) {
            DrawEntityNode(editor, scene, entityID);
        }
    }
}

void Editor::UI::SceneHierarchy::DrawEntityNode(
    VeeEditor *editor,
    const std::shared_ptr<Scene> &scene,
    const EntityID entityID
) {
    const auto entityManager = scene->GetEntityManager();
    const auto componentManager = scene->GetComponentManager();

    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow;

    const bool hasChildren = componentManager->HasComponent<ChildrenComponent>(entityID);
    if (!hasChildren) {
        nodeFlags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (entityID == editor->GetSelectedEntity()) {
        nodeFlags |= ImGuiTreeNodeFlags_Selected;
    }

    const bool isInternal = componentManager->HasComponent<InternalTagComponent>(entityID);
    const std::string entityName = entityManager->GetEntityName(entityID);

    const bool nodeOpen = ImGui::TreeNodeEx(
        reinterpret_cast<void *>(static_cast<uintptr_t>(entityID)),
        nodeFlags,
        isInternal ? "%s (Internal)" : "%s",
        Utils::Strings::TruncateString(entityName, 20).c_str()
    );

    if (ImGui::IsItemClicked()) {
        editor->SelectEntity(entityID);
    }

    if (nodeOpen) {
        if (hasChildren) {
            const auto &childrenComp = componentManager->GetComponent<ChildrenComponent>(entityID);

            for (const EntityID childID: childrenComp.children) {
                DrawEntityNode(editor, scene, childID);
            }
        }

        ImGui::TreePop();
    }
}

void Editor::UI::SceneHierarchy::DrawScenePicker(VeeEditor *editor, const std::shared_ptr<Scene> &scene) {
    const float availWidth = ImGui::GetContentRegionAvail().x;
    const ImGuiStyle &style = ImGui::GetStyle();
    const char *currentName = scene->GetName().c_str();

    const float plusTextW = ImGui::CalcTextSize("+").x;
    float buttonWidth = plusTextW + style.FramePadding.x * 2.0f;
    if (buttonWidth < 26.0f) buttonWidth = 26.0f;
    const float spacing = style.ItemInnerSpacing.x;

    float comboWidth = availWidth - buttonWidth - spacing;
    if (comboWidth < 0.0f) comboWidth = 0.0f;
    ImGui::SetNextItemWidth(comboWidth);

    if (ImGui::BeginCombo("##Scenes", currentName)) {
        for (const auto &sceneData: SceneManager::ListScenes()) {
            const bool isSelected = (scene->GetName() == sceneData.name);
            if (ImGui::Selectable(sceneData.name.c_str(), isSelected)) {
                editor->LoadScene(sceneData.path);
            }
            if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine(0.0f, spacing);
    if (ImGui::Button("+", ImVec2(buttonWidth, 0.0f))) {
        editor->NewEmptyScene();
    }
}

void Editor::UI::SceneHierarchy::Draw(const char *title, VeeEditor *editor) {
    ImGui::Begin(title);

    const auto currentScene = editor->GetScene();

    DrawScenePicker(editor, currentScene);
    ImGui::Separator();
    DrawHierarchy(editor, currentScene);

    ImGui::End();
}
