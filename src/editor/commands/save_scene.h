#ifndef VEE_SAVE_SCENE_H
#define VEE_SAVE_SCENE_H
#include "editor_command.h"
#include "../ui/interface/modals/save_scene_as_modal.h"

class VeeEditor;

namespace Editor::Command {
    class SaveScene final : public IEditorCommand {
        VeeEditor *m_Editor;

    public:
        explicit SaveScene(VeeEditor *editor);

        void Execute() override;;
    };
}

#endif //VEE_SAVE_SCENE_H
