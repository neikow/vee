#ifndef GAME_ENGINE_EDITOR_H
#define GAME_ENGINE_EDITOR_H
#include <memory>

#include "../engine/engine.h"
#include "scenes/scene_manager.h"
#include "settings/editor_settings.h"

struct EditorState {
    bool isViewportFocused = false;
    bool isViewportHovered = false;
    glm::vec<2, uint32_t> viewportSize = {0, 0};
};

class Editor {
    std::shared_ptr<Engine> m_Engine;
    std::unique_ptr<SceneManager> m_SceneManager;

    EditorSettings m_EditorSettings;
    EditorState m_State;

    EntityID m_SelectedEntity = NULL_ENTITY;

    void DrawCurrentSceneHierarchy();

    void DrawScenePicker();

    void DrawSceneHierarchy();

    void DrawInspector();

    void ReloadCurrentSceneFromFile();

    void HandleEntitySelectionWithinViewport(double normX, double normY);

    void DrawViewport();

    void DrawAssetManager() const;

    void DrawUI();

public:
    explicit Editor(const std::shared_ptr<Engine> &engine)
        : m_Engine(engine) {
        m_SceneManager = std::make_unique<SceneManager>(m_Engine);
    }

    [[nodiscard]] EditorSettings GetEditorSettings() const {
        return m_EditorSettings;
    }

    [[nodiscard]] EditorState GetEditorState() const {
        return m_State;
    }

    void SelectEntity(const EntityID entityID) {
        m_SelectedEntity = entityID;
    }

    void Run(int width, int height);

    void CreateEditorCamera(const std::shared_ptr<Scene> &scene) const;

    void CreateEditorInternalEntities() const;

    void LoadScene(const std::string &path) const {
        m_Engine->Pause();
        m_SceneManager->LoadScene(path);
        CreateEditorInternalEntities();
    };
};


#endif //GAME_ENGINE_EDITOR_H
