#ifndef VEE_WINDOW_H
#define VEE_WINDOW_H
#include <string>
#include <GLFW/glfw3.h>

#include "../io/input_system.h"


class Window {
    GLFWwindow *m_Window = nullptr;

public:
    Window(
        int width,
        int height,
        const std::string &title
    );

    void SetResizeCallback(GLFWframebuffersizefun callback) const;

    GLFWwindow *GetWindow() const;

    void Destroy() const;;

    bool ShouldClose() const;
};


#endif
