#include "Input.h"
#include <cstring>
#include <cmath>
#include <algorithm>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
typedef struct _XINPUT_VIBRATION {
    WORD wLeftMotorSpeed;
    WORD wRightMotorSpeed;
} XINPUT_VIBRATION;
typedef DWORD (WINAPI *PFN_XInputSetState)(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);
static PFN_XInputSetState pfnXInputSetState = nullptr;
static HMODULE hXInputDll = nullptr;
#endif

bool Input::currentKeys[GLFW_KEY_LAST + 1] = {false};
bool Input::justPressedKeys[GLFW_KEY_LAST + 1] = {false};
bool Input::justReleasedKeys[GLFW_KEY_LAST + 1] = {false};
bool Input::doubleTapKeys[GLFW_KEY_LAST + 1] = {false};
float Input::lastPressTime[GLFW_KEY_LAST + 1] = {0.0f};
float Input::currentTime = 0.0f;

bool Input::gamepadConnected = false;
int Input::activeGamepadJid = -1;
unsigned char Input::currentGamepadButtons[GLFW_GAMEPAD_BUTTON_LAST + 1] = {0};
unsigned char Input::justPressedGamepadButtons[GLFW_GAMEPAD_BUTTON_LAST + 1] = {0};
unsigned char Input::justReleasedGamepadButtons[GLFW_GAMEPAD_BUTTON_LAST + 1] = {0};
float Input::gamepadAxes[GLFW_GAMEPAD_AXIS_LAST + 1] = {0.0f};
bool Input::lastInputGamepad = false;
float Input::rumbleTimer = 0.0f;

void Input::Init(GLFWwindow* window) {
    std::memset(currentKeys, 0, sizeof(currentKeys));
    std::memset(justPressedKeys, 0, sizeof(justPressedKeys));
    std::memset(justReleasedKeys, 0, sizeof(justReleasedKeys));
    std::memset(doubleTapKeys, 0, sizeof(doubleTapKeys));
    std::memset(lastPressTime, 0, sizeof(lastPressTime));
    currentTime = 0.0f;

    std::memset(currentGamepadButtons, 0, sizeof(currentGamepadButtons));
    std::memset(justPressedGamepadButtons, 0, sizeof(justPressedGamepadButtons));
    std::memset(justReleasedGamepadButtons, 0, sizeof(justReleasedGamepadButtons));
    std::memset(gamepadAxes, 0, sizeof(gamepadAxes));
    gamepadConnected = false;
    activeGamepadJid = -1;
    lastInputGamepad = false;
    rumbleTimer = 0.0f;

    glfwSetKeyCallback(window, KeyCallback);

#ifdef _WIN32
    if (!hXInputDll) {
        const char* dlls[] = {"xinput1_4.dll", "xinput9_1_0.dll", "xinput1_3.dll"};
        for (const char* dll : dlls) {
            hXInputDll = LoadLibraryA(dll);
            if (hXInputDll) {
                pfnXInputSetState = (PFN_XInputSetState)GetProcAddress(hXInputDll, "XInputSetState");
                if (pfnXInputSetState) break;
                FreeLibrary(hXInputDll);
                hXInputDll = nullptr;
            }
        }
    }
#endif
}

void Input::KeyCallback(GLFWwindow*, int key, int, int action, int) {
    if (key >= 0 && key <= GLFW_KEY_LAST) {
        if (action == GLFW_PRESS) {
            currentKeys[key] = true;
            justPressedKeys[key] = true;
            lastInputGamepad = false;

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

    // Detect connected gamepad
    gamepadConnected = false;
    activeGamepadJid = -1;
    for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_4; ++jid) {
        if (glfwJoystickPresent(jid) && glfwJoystickIsGamepad(jid)) {
            activeGamepadJid = jid;
            gamepadConnected = true;
            break;
        }
    }

    if (gamepadConnected && activeGamepadJid >= 0) {
        GLFWgamepadstate state;
        if (glfwGetGamepadState(activeGamepadJid, &state)) {
            // Update buttons
            for (int i = 0; i <= GLFW_GAMEPAD_BUTTON_LAST; ++i) {
                unsigned char pressed = state.buttons[i];
                if (pressed && !currentGamepadButtons[i]) {
                    justPressedGamepadButtons[i] = 1;
                    lastInputGamepad = true;
                } else if (!pressed && currentGamepadButtons[i]) {
                    justReleasedGamepadButtons[i] = 1;
                }
                currentGamepadButtons[i] = pressed;
            }

            // Update axes
            for (int i = 0; i <= GLFW_GAMEPAD_AXIS_LAST; ++i) {
                gamepadAxes[i] = state.axes[i];
                if (std::abs(gamepadAxes[i]) > 0.25f) {
                    lastInputGamepad = true;
                }
            }
        }
    }

    UpdateRumble(dt);
}

void Input::EndFrame() {
    std::memset(justPressedKeys, 0, sizeof(justPressedKeys));
    std::memset(justReleasedKeys, 0, sizeof(justReleasedKeys));
    std::memset(doubleTapKeys, 0, sizeof(doubleTapKeys));

    std::memset(justPressedGamepadButtons, 0, sizeof(justPressedGamepadButtons));
    std::memset(justReleasedGamepadButtons, 0, sizeof(justReleasedGamepadButtons));
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

bool Input::IsGamepadConnected() {
    return gamepadConnected;
}

const char* Input::GetGamepadName() {
    if (gamepadConnected && activeGamepadJid >= 0) {
        return glfwGetGamepadName(activeGamepadJid);
    }
    return "None";
}

float Input::GetGamepadAxis(int axis, float deadzone) {
    if (!gamepadConnected || axis < 0 || axis > GLFW_GAMEPAD_AXIS_LAST) return 0.0f;
    float val = gamepadAxes[axis];
    if (std::abs(val) < deadzone) return 0.0f;
    // Rescale smoothly from deadzone to 1.0
    float sign = (val > 0.0f) ? 1.0f : -1.0f;
    return sign * ((std::abs(val) - deadzone) / (1.0f - deadzone));
}

bool Input::IsGamepadButtonDown(int button) {
    if (!gamepadConnected || button < 0 || button > GLFW_GAMEPAD_BUTTON_LAST) return false;
    return currentGamepadButtons[button] != 0;
}

bool Input::IsGamepadButtonPressed(int button) {
    if (!gamepadConnected || button < 0 || button > GLFW_GAMEPAD_BUTTON_LAST) return false;
    return justPressedGamepadButtons[button] != 0;
}

bool Input::IsGamepadButtonReleased(int button) {
    if (!gamepadConnected || button < 0 || button > GLFW_GAMEPAD_BUTTON_LAST) return false;
    return justReleasedGamepadButtons[button] != 0;
}

float Input::GetAxisHorizontal() {
    float kx = 0.0f;
    if (IsKeyDown(GLFW_KEY_A) || IsKeyDown(GLFW_KEY_LEFT)) kx -= 1.0f;
    if (IsKeyDown(GLFW_KEY_D) || IsKeyDown(GLFW_KEY_RIGHT)) kx += 1.0f;

    if (gamepadConnected) {
        float gx = GetGamepadAxis(GLFW_GAMEPAD_AXIS_LEFT_X, 0.18f);
        if (IsGamepadButtonDown(GLFW_GAMEPAD_BUTTON_DPAD_LEFT)) gx = -1.0f;
        if (IsGamepadButtonDown(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT)) gx = 1.0f;
        if (std::abs(gx) > std::abs(kx)) return gx;
    }
    return kx;
}

float Input::GetAxisVertical() {
    float ky = 0.0f;
    if (IsKeyDown(GLFW_KEY_W) || IsKeyDown(GLFW_KEY_UP)) ky += 1.0f;
    if (IsKeyDown(GLFW_KEY_S) || IsKeyDown(GLFW_KEY_DOWN)) ky -= 1.0f;

    if (gamepadConnected) {
        // In GLFW, pushing stick up yields -1.0, so invert for positive = up
        float gy = -GetGamepadAxis(GLFW_GAMEPAD_AXIS_LEFT_Y, 0.18f);
        if (IsGamepadButtonDown(GLFW_GAMEPAD_BUTTON_DPAD_UP)) gy = 1.0f;
        if (IsGamepadButtonDown(GLFW_GAMEPAD_BUTTON_DPAD_DOWN)) gy = -1.0f;
        if (std::abs(gy) > std::abs(ky)) return gy;
    }
    return ky;
}

bool Input::IsFireDown() {
    if (IsKeyDown(GLFW_KEY_SPACE)) return true;
    if (gamepadConnected) {
        if (IsGamepadButtonDown(GLFW_GAMEPAD_BUTTON_A)) return true;
        if (gamepadAxes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] > 0.25f) return true;
    }
    return false;
}

bool Input::IsFirePressed() {
    if (IsKeyPressed(GLFW_KEY_SPACE)) return true;
    if (gamepadConnected) {
        if (IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_A)) return true;
    }
    return false;
}

bool Input::IsFireReleased() {
    if (IsKeyReleased(GLFW_KEY_SPACE)) return true;
    if (gamepadConnected) {
        if (IsGamepadButtonReleased(GLFW_GAMEPAD_BUTTON_A)) return true;
    }
    return false;
}

bool Input::IsBombPressed() {
    if (IsKeyPressed(GLFW_KEY_B)) return true;
    if (gamepadConnected) {
        if (IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_B)) return true;
    }
    return false;
}

bool Input::IsBoostDown() {
    if (IsKeyDown(GLFW_KEY_LEFT_SHIFT) || IsKeyDown(GLFW_KEY_RIGHT_SHIFT)) return true;
    if (gamepadConnected) {
        if (IsGamepadButtonDown(GLFW_GAMEPAD_BUTTON_X)) return true;
        if (gamepadAxes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] > 0.25f && !IsFireDown()) return true;
    }
    return false;
}

bool Input::IsBoostPressed() {
    if (IsKeyPressed(GLFW_KEY_LEFT_SHIFT) || IsKeyPressed(GLFW_KEY_RIGHT_SHIFT)) return true;
    if (gamepadConnected) {
        if (IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_X)) return true;
    }
    return false;
}

bool Input::IsBrakeDown() {
    if (IsKeyDown(GLFW_KEY_LEFT_CONTROL) || IsKeyDown(GLFW_KEY_RIGHT_CONTROL) || IsKeyDown(GLFW_KEY_LEFT_ALT)) return true;
    if (gamepadConnected) {
        if (gamepadAxes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] > 0.25f) return true;
    }
    return false;
}

bool Input::IsBrakePressed() {
    if (IsKeyPressed(GLFW_KEY_LEFT_CONTROL) || IsKeyPressed(GLFW_KEY_RIGHT_CONTROL) || IsKeyPressed(GLFW_KEY_LEFT_ALT)) return true;
    return false;
}

bool Input::IsRollLeftPressed() {
    if (IsDoubleTap(GLFW_KEY_A) || IsDoubleTap(GLFW_KEY_LEFT) ||
        IsKeyPressed(GLFW_KEY_Q) || IsKeyPressed(GLFW_KEY_Z)) return true;
    if (gamepadConnected) {
        if (IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_LEFT_BUMPER)) return true;
    }
    return false;
}

bool Input::IsRollRightPressed() {
    if (IsDoubleTap(GLFW_KEY_D) || IsDoubleTap(GLFW_KEY_RIGHT) ||
        IsKeyPressed(GLFW_KEY_E) || IsKeyPressed(GLFW_KEY_C)) return true;
    if (gamepadConnected) {
        if (IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER)) return true;
    }
    return false;
}

bool Input::IsSomersaultPressed() {
    if (IsKeyPressed(GLFW_KEY_X) || IsKeyPressed(GLFW_KEY_F)) return true;
    if (gamepadConnected) {
        if (IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_Y)) return true;
    }
    return false;
}

bool Input::IsUTurnPressed() {
    return false;
}

bool Input::IsCockpitTogglePressed() {
    if (IsKeyPressed(GLFW_KEY_V)) return true;
    if (gamepadConnected) {
        if (IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_BACK) ||
            IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_RIGHT_THUMB)) return true;
    }
    return false;
}

bool Input::IsMenuUpPressed() {
    if (IsKeyPressed(GLFW_KEY_UP) || IsKeyPressed(GLFW_KEY_W)) return true;
    if (gamepadConnected) {
        if (IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_UP)) return true;
    }
    return false;
}

bool Input::IsMenuDownPressed() {
    if (IsKeyPressed(GLFW_KEY_DOWN) || IsKeyPressed(GLFW_KEY_S)) return true;
    if (gamepadConnected) {
        if (IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_DOWN)) return true;
    }
    return false;
}

bool Input::IsMenuLeftPressed() {
    if (IsKeyPressed(GLFW_KEY_LEFT) || IsKeyPressed(GLFW_KEY_A)) return true;
    if (gamepadConnected) {
        if (IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_LEFT)) return true;
    }
    return false;
}

bool Input::IsMenuRightPressed() {
    if (IsKeyPressed(GLFW_KEY_RIGHT) || IsKeyPressed(GLFW_KEY_D)) return true;
    if (gamepadConnected) {
        if (IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT)) return true;
    }
    return false;
}

bool Input::IsMenuConfirmPressed() {
    if (IsKeyPressed(GLFW_KEY_ENTER) || IsKeyPressed(GLFW_KEY_SPACE)) return true;
    if (gamepadConnected) {
        if (IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_A) ||
            IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_START)) return true;
    }
    return false;
}

bool Input::IsMenuCancelPressed() {
    if (IsKeyPressed(GLFW_KEY_ESCAPE)) return true;
    if (gamepadConnected) {
        if (IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_B)) return true;
    }
    return false;
}

bool Input::IsLastInputGamepad() {
    return gamepadConnected && lastInputGamepad;
}

void Input::SetRumble(float lowFreqMotor, float highFreqMotor, float durationSec) {
#ifdef _WIN32
    if (!pfnXInputSetState) return;
    XINPUT_VIBRATION vib;
    vib.wLeftMotorSpeed = static_cast<WORD>(std::clamp(lowFreqMotor, 0.0f, 1.0f) * 65535.0f);
    vib.wRightMotorSpeed = static_cast<WORD>(std::clamp(highFreqMotor, 0.0f, 1.0f) * 65535.0f);
    pfnXInputSetState(0, &vib);
    rumbleTimer = durationSec;
#else
    (void)lowFreqMotor; (void)highFreqMotor; (void)durationSec;
#endif
}

void Input::StopRumble() {
#ifdef _WIN32
    if (!pfnXInputSetState) return;
    XINPUT_VIBRATION vib;
    vib.wLeftMotorSpeed = 0;
    vib.wRightMotorSpeed = 0;
    pfnXInputSetState(0, &vib);
    rumbleTimer = 0.0f;
#endif
}

void Input::UpdateRumble(float dt) {
#ifdef _WIN32
    if (rumbleTimer > 0.0f) {
        rumbleTimer -= dt;
        if (rumbleTimer <= 0.0f) {
            StopRumble();
        }
    }
#else
    (void)dt;
#endif
}

