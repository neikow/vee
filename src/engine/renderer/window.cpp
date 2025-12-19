#include "window.h"

Window::Window(const int width, const int height, const std::string &title) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_Window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

    glfwSetWindowUserPointer(m_Window, this);

    InputSystem::SetupCallbacks(m_Window);
}

void Window::SetResizeCallback(const GLFWframebuffersizefun callback) const {
    glfwSetFramebufferSizeCallback(m_Window, callback);
}

GLFWwindow *Window::GetWindow() const {
    return m_Window;
}

void Window::Destroy() const {
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

Extent &Window::GetExtent() const {
    int width, height;
    glfwGetFramebufferSize(m_Window, &width, &height);
    static Extent extent{
        .width = width,
        .height = height
    };
    return extent;
}

bool Window::ShouldClose() const {
    return glfwWindowShouldClose(m_Window);
}
