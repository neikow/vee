#ifndef VEE_INPUT_SYSTEM_H
#define VEE_INPUT_SYSTEM_H

#include <map>
#include <GLFW/glfw3.h>

enum Key {
    KEY_A,
    KEY_Z,
    KEY_Q,
    KEY_S,
    KEY_D,
    KEY_R,
    KEY_LEFT_CONTROL,
    KEY_SPACE,
    KEY_SUPER,
    KEY_DELETE,
};

enum MouseButton {
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_RIGHT,
};

class InputSystem {
    static std::map<Key, bool> s_keys;
    static std::map<MouseButton, bool> s_mouseButtons;
    static float s_scrollDeltaY;
    static double s_mouseX;
    static double s_mouseY;

    InputSystem() = default;

    ~InputSystem() = default;

public:
    static bool IsKeyPressed(Key key);

    static bool IsMouseButtonPressed(MouseButton button);

    static float GetScrollDeltaY();

    static double GetMouseX();

    static double GetMouseY();

    static void SetKeyState(Key key, bool pressed);

    static void SetMouseButtonState(MouseButton button, bool pressed);

    static void SetScrollDelta(float deltaY);

    static void SetMousePosition(double x, double y);

    static void UpdateEndOfFrame();

    static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

    static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);

    static void CursorPositionCallback(GLFWwindow *window, double xPos, double yPos);

    static void ScrollCallback(GLFWwindow *window, double xOffset, double yOffset);

    static void SetupCallbacks(GLFWwindow *window);
};

#endif //VEE_INPUT_SYSTEM_H
