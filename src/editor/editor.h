#ifndef GAME_ENGINE_EDITOR_H
#define GAME_ENGINE_EDITOR_H
#include <memory>

#include "../engine/engine.h"
#include "io/shortcut_manager.h"
#include "scenes/scene_manager.h"
#include "settings/editor_settings.h"
#include "ui/editor_console.h"

struct EditorState {
    bool isViewportFocused = false;
    bool isViewportHovered = false;
    glm::vec<2, uint32_t> viewportSize = {0, 0};
    bool shouldSaveSceneAsModalOpen = false;
};

class VeeEditor {
    std::shared_ptr<Engine> m_Engine;
    std::unique_ptr<SceneManager> m_SceneManager;

    std::unique_ptr<ShortcutManager> m_ShortcutManager;

    Signature m_EditorCameraSignature;

    EditorSettings m_EditorSettings;
    EditorState m_State;

    EntityID m_SelectedEntity = NULL_ENTITY;

    void DrawModals();

    void DrawUI();

public:
    explicit VeeEditor(const std::shared_ptr<Engine> &engine)
        : m_Engine(engine) {
        m_SceneManager = std::make_unique<SceneManager>(m_Engine);
        m_ShortcutManager = std::make_unique<ShortcutManager>();
    }

    void ReloadCurrentSceneFromFile();

    void HandleEntitySelectionWithinViewport(double normX, double normY);

    void NewEmptyScene();

    [[nodiscard]] EditorSettings GetEditorSettings() const {
        return m_EditorSettings;
    }

    [[nodiscard]] EditorSettings &GetEditorSettings() {
        return m_EditorSettings;
    }

    [[nodiscard]] EditorState GetEditorState() const {
        return m_State;
    }

    [[nodiscard]] EditorState &GetEditorState() {
        return m_State;
    }

    void SelectEntity(const EntityID entityID) {
        m_SelectedEntity = entityID;
    }

    void Run(int width, int height);

    void CreateEditorCamera(const std::shared_ptr<Scene> &scene);

    void CreateEditorInternalEntities();

    void LoadScene(const std::string &path) {
        m_Engine->Pause();
        m_SceneManager->LoadScene(path);
        CreateEditorInternalEntities();
    }

    [[nodiscard]] EntityID GetSelectedEntity() const {
        return m_SelectedEntity;
    }

    [[nodiscard]] std::shared_ptr<Scene> GetScene() const {
        return m_Engine->GetScene();
    }

    [[nodiscard]] std::shared_ptr<Engine> GetEngine() const {
        return m_Engine;
    }
};


#endif //GAME_ENGINE_EDITOR_H
