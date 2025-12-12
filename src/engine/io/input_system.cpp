#include "input_system.h"
#include <GLFW/glfw3.h>

std::map<Key, bool> InputSystem::s_keys;
std::map<MouseButton, bool> InputSystem::s_mouseButtons;
float InputSystem::s_scrollDeltaY = 0.0f;
double InputSystem::s_mouseX = 0.0;
double InputSystem::s_mouseY = 0.0;

bool InputSystem::IsKeyPressed(const Key key) {
    if (s_keys.contains(key)) {
        return s_keys[key];
    }
    return false;
}

bool InputSystem::IsMouseButtonPressed(const MouseButton button) {
    if (s_mouseButtons.contains(button)) {
        return s_mouseButtons[button];
    }
    return false;
}

float InputSystem::GetScrollDeltaY() {
    return s_scrollDeltaY;
}

double InputSystem::GetMouseX() {
    return s_mouseX;
}

double InputSystem::GetMouseY() {
    return s_mouseY;
}

void InputSystem::SetKeyState(const Key key, const bool pressed) {
    s_keys[key] = pressed;
}

void InputSystem::SetMouseButtonState(const MouseButton button, bool pressed) {
    s_mouseButtons[button] = pressed;
}

void InputSystem::SetScrollDelta(const float deltaY) {
    s_scrollDeltaY += deltaY;
}

void InputSystem::SetMousePosition(double x, double y) {
    s_mouseX = x;
    s_mouseY = y;
}

void InputSystem::UpdateEndOfFrame() {
    s_scrollDeltaY = 0.0f;
}

static bool isPressed(const int action) {
    return action == GLFW_PRESS || action == GLFW_REPEAT;
}

void InputSystem::KeyCallback(GLFWwindow */*window*/, const int key, int /*scancode*/, const int action, int /*mods*/) {
    Key mappedKey;
    switch (key) {
        case GLFW_KEY_Z:
            mappedKey = KEY_Z;
            break;
        case GLFW_KEY_Q:
            mappedKey = KEY_Q;
            break;
        case GLFW_KEY_S:
            mappedKey = KEY_S;
            break;
        case GLFW_KEY_D:
            mappedKey = KEY_D;
            break;
        case GLFW_KEY_LEFT_CONTROL:
            mappedKey = KEY_LEFT_CONTROL;
            break;
        case GLFW_KEY_SPACE:
            mappedKey = KEY_SPACE;
            break;
        case GLFW_KEY_R:
            mappedKey = KEY_R;
            break;
        case GLFW_KEY_LEFT_SUPER:
            mappedKey = KEY_SUPER;
            break;
        case GLFW_KEY_A:
            mappedKey = KEY_A;
            break;
        case GLFW_KEY_DELETE:
            mappedKey = KEY_DELETE;
            break;
        default:
            return;
    }
    SetKeyState(mappedKey, isPressed(action));
}

void InputSystem::MouseButtonCallback(GLFWwindow */*window*/, int button, const int action, int /*mods*/) {
    SetMouseButtonState(static_cast<MouseButton>(button), isPressed(action));
}

void InputSystem::CursorPositionCallback(GLFWwindow */*window*/, const double xPos, const double yPos) {
    SetMousePosition(xPos, yPos);
}

void InputSystem::ScrollCallback(GLFWwindow */*window*/, double /*xOffset*/, const double yOffset) {
    SetScrollDelta(static_cast<float>(yOffset));
}

void InputSystem::SetupCallbacks(GLFWwindow *window) {
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPositionCallback);
    glfwSetScrollCallback(window, ScrollCallback);
}
