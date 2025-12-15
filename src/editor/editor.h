#ifndef GAME_ENGINE_EDITOR_H
#define GAME_ENGINE_EDITOR_H
#include <memory>

#include "../engine/engine.h"
#include "io/shortcut_manager.h"
#include "scenes/scene_manager.h"
#include "settings/editor_settings.h"
#include "ui/editor_ui_manager.h"

/** Struct representing the current state of the editor.
 */
struct EditorState {
    bool isViewportFocused = false;
    bool isViewportHovered = false;
    glm::vec<2, uint32_t> viewportSize = {0, 0};
};

/** Main class representing the game engine editor application.
 */
class VeeEditor {
    /** Core engine instance used by the editor.
     */
    std::shared_ptr<Engine> m_Engine;

    /** Manages scenes within the editor.
     */
    std::shared_ptr<SceneManager> m_SceneManager;
    /** Manages keyboard shortcuts within the editor.
     */
    std::unique_ptr<ShortcutManager> m_ShortcutManager;
    /** Manages the editor's user interface.
     */
    std::shared_ptr<Editor::UIManager> m_UIManager;

    /** Signature used to identify the editor camera entity.
     */
    Signature m_EditorCameraSignature;

    /** Current settings of the editor.
     */
    EditorSettings m_EditorSettings;
    /** Current state of the editor.
     */
    EditorState m_State;

    /** Currently selected entity ID within the editor.
     * NULL_ENTITY indicates no selection.
     */
    EntityID m_SelectedEntity = NULL_ENTITY;

    /** Renders modal dialogs within the editor UI.
     */
    void DrawModals() const;

    /** Renders the entire editor UI.
     */
    void DrawUI();

    /** Registers keyboard shortcuts for editor commands.
     */
    void RegisterShortcuts();

    /** Registers internal systems required for the editor's functionality.
     */
    void RegisterInternalSystems();

    /** Creates the editor camera entity within the specified scene.
     *
     * @param scene Shared pointer to the scene where the editor camera will be created.
     */
    void CreateEditorCamera(const std::shared_ptr<Scene> &scene);

    /** Creates internal entities required for the editor's functionality.
     */
    void CreateEditorInternalEntities();

public:
    explicit VeeEditor(const std::shared_ptr<Engine> &engine);

    void HandleEntitySelectionWithinViewport(double normX, double normY);

    void NewEmptyScene();

    /** Retrieves the current editor settings.
     *
     * @return A copy of the current EditorSettings.
     */
    [[nodiscard]] EditorSettings GetEditorSettings() const;

    /** Retrieves a reference to the current editor settings.
     *
     * @return Reference to the current EditorSettings.
     */
    [[nodiscard]] EditorSettings &GetEditorSettings();

    /** Retrieves the current editor state.
     *
     * @return A copy of the current EditorState.
     */
    [[nodiscard]] EditorState GetEditorState() const;

    /** Retrieves a reference to the current editor state.
     *
     * @return Reference to the current EditorState.
     */
    [[nodiscard]] EditorState &GetEditorState();

    /** Selects an entity by its ID.
     *
     * @param entityID The ID of the entity to select.
     */
    void SelectEntity(EntityID entityID);

    /** Loads a scene from the specified file path.
     *
     * @param path The file path of the scene to load.
     */
    void LoadScene(const std::string &path);

    /** Retrieves the currently selected entity ID.
     *
     * @return The ID of the selected entity, or NULL_ENTITY if none is selected.
     */
    [[nodiscard]] EntityID GetSelectedEntity() const;

    /** Retrieves the current Scene instance.
     *
     * @return Shared pointer to the current Scene.
     */
    [[nodiscard]] std::shared_ptr<Scene> GetScene() const;

    /** Retrieves the Engine instance.
     *
     * @return Shared pointer to the Engine.
     */
    [[nodiscard]] std::shared_ptr<Engine> GetEngine() const;

    /** Retrieves the UIManager instance.
     *
     * @return Shared pointer to the UIManager.
     */
    [[nodiscard]] std::shared_ptr<Editor::UIManager> GetUIManager() const;

    /** Retrieves the SceneManager instance.
     *
     * @return Shared pointer to the SceneManager.
     */
    std::shared_ptr<SceneManager> GetSceneManager();

    /** Main loop of the editor application.
     *
     * @param width Initial width of the application window.
     * @param height Initial height of the application window.
     */
    void Run(int width, int height);

    /** Initializes the editor, setting up UI, shortcuts, and internal systems.
     */
    void Initialize();
};


#endif //GAME_ENGINE_EDITOR_H
