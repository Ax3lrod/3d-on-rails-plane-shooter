#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>

class Input {
public:
    static void Init(GLFWwindow* window);
    static void Update(float dt);
    static void EndFrame();

    static bool IsKeyDown(int key);
    static bool IsKeyPressed(int key);
    static bool IsKeyReleased(int key);
    static bool IsDoubleTap(int key);

private:
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    static bool currentKeys[GLFW_KEY_LAST + 1];
    static bool justPressedKeys[GLFW_KEY_LAST + 1];
    static bool justReleasedKeys[GLFW_KEY_LAST + 1];
    static bool doubleTapKeys[GLFW_KEY_LAST + 1];
    static float lastPressTime[GLFW_KEY_LAST + 1];
    static float currentTime;
    static constexpr float DOUBLE_TAP_THRESHOLD = 0.38f; // Seconds for double-tap
};

#endif
