#ifndef VEE_DELETE_ENTITY_H
#define VEE_DELETE_ENTITY_H
#include "editor_command.h"

class VeeEditor;

namespace Editor::Command {
    class DeleteEntity final : public IEditorCommand {
        VeeEditor *m_Editor;

    public:
        explicit DeleteEntity(VeeEditor *editor);

        void Execute() override;;
    };
}


#endif //VEE_DELETE_ENTITY_H
