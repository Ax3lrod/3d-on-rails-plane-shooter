#include "Input.h"
#include <cstring>

bool Input::currentKeys[GLFW_KEY_LAST + 1] = {false};
bool Input::justPressedKeys[GLFW_KEY_LAST + 1] = {false};
bool Input::justReleasedKeys[GLFW_KEY_LAST + 1] = {false};
bool Input::doubleTapKeys[GLFW_KEY_LAST + 1] = {false};
float Input::lastPressTime[GLFW_KEY_LAST + 1] = {0.0f};
float Input::currentTime = 0.0f;

void Input::Init(GLFWwindow* window) {
    std::memset(currentKeys, 0, sizeof(currentKeys));
    std::memset(justPressedKeys, 0, sizeof(justPressedKeys));
    std::memset(justReleasedKeys, 0, sizeof(justReleasedKeys));
    std::memset(doubleTapKeys, 0, sizeof(doubleTapKeys));
    std::memset(lastPressTime, 0, sizeof(lastPressTime));
    currentTime = 0.0f;

    glfwSetKeyCallback(window, KeyCallback);
}

void Input::KeyCallback(GLFWwindow*, int key, int, int action, int) {
    if (key >= 0 && key <= GLFW_KEY_LAST) {
        if (action == GLFW_PRESS) {
            currentKeys[key] = true;
            justPressedKeys[key] = true;

            float timeSinceLast = currentTime - lastPressTime[key];
            if (timeSinceLast > 0.02f && timeSinceLast <= DOUBLE_TAP_THRESHOLD) {
                doubleTapKeys[key] = true;
            }
            lastPressTime[key] = currentTime;
        } else if (action == GLFW_RELEASE) {
            currentKeys[key] = false;
            justReleasedKeys[key] = true;
        }
    }
}

void Input::Update(float dt) {
    currentTime += dt;
}

void Input::EndFrame() {
    std::memset(justPressedKeys, 0, sizeof(justPressedKeys));
    std::memset(justReleasedKeys, 0, sizeof(justReleasedKeys));
    std::memset(doubleTapKeys, 0, sizeof(doubleTapKeys));
}

bool Input::IsKeyDown(int key) {
    if (key >= 0 && key <= GLFW_KEY_LAST) {
        return currentKeys[key];
    }
    return false;
}

bool Input::IsKeyPressed(int key) {
    if (key >= 0 && key <= GLFW_KEY_LAST) {
        return justPressedKeys[key];
    }
    return false;
}

bool Input::IsKeyReleased(int key) {
    if (key >= 0 && key <= GLFW_KEY_LAST) {
        return justReleasedKeys[key];
    }
    return false;
}

bool Input::IsDoubleTap(int key) {
    if (key >= 0 && key <= GLFW_KEY_LAST) {
        return doubleTapKeys[key];
    }
    return false;
}
