#ifndef VEE_CONSOLE_H
#define VEE_CONSOLE_H
#include "imgui.h"
#include "../../../engine/logging/logger.h"

ImVec4 GetLogLevelColor(LogLevel level);

class VeeEditor;

namespace Editor::UI {
    class Console {
    public:
        /** Draws the Console window.
         *
         * @param title The title of the window.
         * @param editor Pointer to the VeeEditor instance.
        */
        static void Draw(const char *title, VeeEditor *editor);
    };
}
#endif //VEE_CONSOLE_H
