#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>

class Input {
public:
    static void Init(GLFWwindow* window);
    static void Update(float dt);
    static void EndFrame();

    // Keyboard Low-Level
    static bool IsKeyDown(int key);
    static bool IsKeyPressed(int key);
    static bool IsKeyReleased(int key);
    static bool IsDoubleTap(int key);

    // Gamepad Low-Level
    static bool IsGamepadConnected();
    static const char* GetGamepadName();
    static float GetGamepadAxis(int axis, float deadzone = 0.18f);
    static bool IsGamepadButtonDown(int button);
    static bool IsGamepadButtonPressed(int button);
    static bool IsGamepadButtonReleased(int button);

    // High-Level Unified Action Queries (Seamlessly merging Keyboard & Gamepad)
    static float GetAxisHorizontal(); // -1.0 (Left) to +1.0 (Right)
    static float GetAxisVertical();   // -1.0 (Down) to +1.0 (Up)
    static bool IsFireDown();
    static bool IsFirePressed();
    static bool IsFireReleased();
    static bool IsBombPressed();
    static bool IsBoostDown();
    static bool IsBoostPressed();
    static bool IsBrakeDown();
    static bool IsBrakePressed();
    static bool IsRollLeftPressed();
    static bool IsRollRightPressed();
    static bool IsSomersaultPressed();
    static bool IsUTurnPressed();
    static bool IsCockpitTogglePressed();

    // Menu Navigation
    static bool IsMenuUpPressed();
    static bool IsMenuDownPressed();
    static bool IsMenuLeftPressed();
    static bool IsMenuRightPressed();
    static bool IsMenuConfirmPressed();
    static bool IsMenuCancelPressed();

    // Prompts
    static bool IsLastInputGamepad();

    // Controller Haptics / Rumble (XInput)
    static void SetRumble(float lowFreqMotor, float highFreqMotor, float durationSec = 0.2f);
    static void StopRumble();

private:
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    static bool currentKeys[GLFW_KEY_LAST + 1];
    static bool justPressedKeys[GLFW_KEY_LAST + 1];
    static bool justReleasedKeys[GLFW_KEY_LAST + 1];
    static bool doubleTapKeys[GLFW_KEY_LAST + 1];
    static float lastPressTime[GLFW_KEY_LAST + 1];
    static float currentTime;
    static constexpr float DOUBLE_TAP_THRESHOLD = 0.38f;

    // Gamepad state
    static bool gamepadConnected;
    static int activeGamepadJid;
    static unsigned char currentGamepadButtons[GLFW_GAMEPAD_BUTTON_LAST + 1];
    static unsigned char justPressedGamepadButtons[GLFW_GAMEPAD_BUTTON_LAST + 1];
    static unsigned char justReleasedGamepadButtons[GLFW_GAMEPAD_BUTTON_LAST + 1];
    static float gamepadAxes[GLFW_GAMEPAD_AXIS_LAST + 1];
    static bool lastInputGamepad;

    // Rumble state
    static float rumbleTimer;
    static void UpdateRumble(float dt);
};

#endif
