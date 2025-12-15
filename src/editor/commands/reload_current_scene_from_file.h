#ifndef VEE_RELOAD_CURRENT_SCENE_FROM_FILE_H
#define VEE_RELOAD_CURRENT_SCENE_FROM_FILE_H
#include "editor_command.h"

class VeeEditor;

namespace Editor::Command {
    /**
     * Command to reload the current scene from its file and discard unsaved changes.
     */
    class ReloadCurrentSceneFromFile final : public IEditorCommand {
        VeeEditor *m_Editor;

    public:
        explicit ReloadCurrentSceneFromFile(VeeEditor *editor);

        void Execute() override;
    };
}
#endif
