#ifndef VEE_VIEWPORT_H
#define VEE_VIEWPORT_H
#include "../../editor.h"

namespace Editor::UI {
    class Viewport {
    public:
        /** Draws the Viewport window.
         *
         * @param title The title of the window.
         * @param editor Pointer to the VeeEditor instance.
         */
        static void Draw(const char *title, VeeEditor *editor);
    };
}

#endif //VEE_VIEWPORT_H
