#include "window.h"

Window::Window(const int width, const int height, const std::string &title) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_Window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

    glfwSetWindowUserPointer(m_Window, this);
    glfwSetFramebufferSizeCallback(m_Window, ResizeCallback);

    InputSystem::SetupCallbacks(m_Window);
}

unsigned long Window::AddResizeCallback(const ResizeCallbackFunction &callback) {
    const unsigned long id = m_ResizeCallbacks.size();
    m_ResizeCallbacks.push_back(callback);
    return id;
}

void Window::RemoveResizeCallback(const unsigned long handle) {
    m_ResizeCallbacks.erase(m_ResizeCallbacks.begin() + handle);
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

void Window::ResizeCallback(
    GLFWwindow *window,
    const int width,
    const int height
) {
    const auto *win = static_cast<Window *>(glfwGetWindowUserPointer(window));
    for (const auto &callback: win->m_ResizeCallbacks) {
        callback(
            Extent{
                .width = width,
                .height = height
            }
        );
    }
}
