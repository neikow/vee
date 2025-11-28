#ifndef GAME_ENGINE_EDITOR_H
#define GAME_ENGINE_EDITOR_H
#include <memory>

#include "../engine/engine.h"
#include "scenes/scene_manager.h"

class Editor {
    std::shared_ptr<Engine> m_Engine;
    std::unique_ptr<SceneManager> m_SceneManager;

    EntityID m_SelectedEntity = 0;

    void DrawSceneHierarchy();

    void DrawInspector();

    void DrawViewport();

    void DrawAssetManager();

public:
    explicit Editor(const std::shared_ptr<Engine> &engine)
        : m_Engine(engine) {
        m_SceneManager = std::make_unique<SceneManager>(m_Engine);
    }

    void Run(int width, int height);
};


#endif //GAME_ENGINE_EDITOR_H
