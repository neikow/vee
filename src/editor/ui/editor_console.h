#ifndef VEE_CONSOLE_H
#define VEE_CONSOLE_H
#include "imgui.h"
#include "../../engine/logging/logger.h"

ImVec4 GetLogLevelColor(LogLevel level);

class EditorConsole {
    bool m_ScrollToBottom = true;

    EditorConsole() = default;

public:
    static EditorConsole &GetInstance() {
        static EditorConsole instance;
        return instance;
    }

    static void Draw(const char *title, bool *p_open);
};

#endif //VEE_CONSOLE_H
