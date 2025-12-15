#ifndef VEE_UI_MODAL_TYPES_H
#define VEE_UI_MODAL_TYPES_H

namespace Editor::UI::Modals {
    /** Interface for editor modal dialogs. */
    class IEditorModal {
    public:
        virtual ~IEditorModal() = default;

        /** Draws the modal dialog UI. */
        virtual void Draw() = 0;
    };
}

#endif
