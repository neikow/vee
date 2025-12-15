#ifndef VEE_CREATE_ENTITY_IN_SCENE_H
#define VEE_CREATE_ENTITY_IN_SCENE_H
#include "editor_command.h"

class VeeEditor;

namespace Editor::Command {
    /**
     * Command to reload the current scene from its file and discard unsaved changes.
     */
    class CreateEntityInScene final : public IEditorCommand {
        VeeEditor *m_Editor;

    public:
        explicit CreateEntityInScene(VeeEditor *editor);

        void Execute() override;
    };
}
#endif
