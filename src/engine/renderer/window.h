#ifndef VEE_WINDOW_H
#define VEE_WINDOW_H
#include <string>
#include <GLFW/glfw3.h>

#include "../io/input_system.h"

struct Extent {
    int width;
    int height;
};

using ResizeCallbackFunction = std::function<void(Extent)>;

class Window {
    GLFWwindow *m_Window = nullptr;

    std::vector<ResizeCallbackFunction> m_ResizeCallbacks;

public:
    Window(
        int width,
        int height,
        const std::string &title
    );

    unsigned long AddResizeCallback(const ResizeCallbackFunction &callback);

    void RemoveResizeCallback(unsigned long handle);

    [[nodiscard]] GLFWwindow *GetWindow() const;

    void Destroy() const;

    [[nodiscard]] Extent &GetExtent() const;

    [[nodiscard]] bool ShouldClose() const;

private:
    static void ResizeCallback(
        GLFWwindow *window,
        int width,
        int height
    );
};


#endif
