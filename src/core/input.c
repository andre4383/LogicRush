#include "input.h"
#include "game.h"
#include "theme.h"
#include "raymath.h"
#include <math.h>

#define GAMEPAD_ID           0
#define STICK_DEADZONE       0.20f
#define CURSOR_SPEED         520.0f
#define RUMBLE_DAMAGE_TIME   0.35f
#define RUMBLE_DAMAGE_STR    0.90f
#define RUMBLE_PHASE_TIME    0.50f
#define RUMBLE_PHASE_STR     0.65f
#define RUMBLE_COMBO_TIME    0.20f
#define RUMBLE_COMBO_STR     0.45f

typedef enum {
    INPUT_DEVICE_MOUSE = 0,
    INPUT_DEVICE_GAMEPAD
} InputDevice;

static InputDevice activeDevice = INPUT_DEVICE_MOUSE;
static Vector2 virtualCursor = { 640.0f, 360.0f };
static Vector2 lastMousePos = { 0.0f, 0.0f };

static bool gamepadConnected(void) {
    return IsGamepadAvailable(GAMEPAD_ID);
}

static float applyDeadzone(float v) {
    if (fabsf(v) < STICK_DEADZONE) return 0.0f;
    float sign = (v > 0.0f) ? 1.0f : -1.0f;
    float norm = (fabsf(v) - STICK_DEADZONE) / (1.0f - STICK_DEADZONE);
    return sign * norm;
}

static bool gamepadAnyActivity(void) {
    if (!gamepadConnected()) return false;

    if (fabsf(GetGamepadAxisMovement(GAMEPAD_ID, GAMEPAD_AXIS_LEFT_X)) > STICK_DEADZONE) return true;
    if (fabsf(GetGamepadAxisMovement(GAMEPAD_ID, GAMEPAD_AXIS_LEFT_Y)) > STICK_DEADZONE) return true;
    if (fabsf(GetGamepadAxisMovement(GAMEPAD_ID, GAMEPAD_AXIS_RIGHT_X)) > STICK_DEADZONE) return true;
    if (fabsf(GetGamepadAxisMovement(GAMEPAD_ID, GAMEPAD_AXIS_RIGHT_Y)) > STICK_DEADZONE) return true;

    for (int b = 0; b <= GAMEPAD_BUTTON_RIGHT_THUMB; b++) {
        if (IsGamepadButtonPressed(GAMEPAD_ID, b) || IsGamepadButtonDown(GAMEPAD_ID, b))
            return true;
    }
    return false;
}

static void startRumble(float duration, float strength) {
    if (!gamepadConnected()) return;
    SetGamepadVibration(GAMEPAD_ID, strength, strength, duration);
}

void Input_Init(void) {
    virtualCursor = (Vector2){ SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f };
    lastMousePos = GetMousePosition();
    activeDevice = INPUT_DEVICE_MOUSE;
}

void Input_Update(float dt) {
    Vector2 mouse = GetMousePosition();
    if (Vector2Distance(mouse, lastMousePos) > 1.0f) {
        activeDevice = INPUT_DEVICE_MOUSE;
    }
    lastMousePos = mouse;

    if (gamepadAnyActivity()) {
        activeDevice = INPUT_DEVICE_GAMEPAD;
    }

    if (activeDevice == INPUT_DEVICE_GAMEPAD && gamepadConnected()) {
        float rx = applyDeadzone(GetGamepadAxisMovement(GAMEPAD_ID, GAMEPAD_AXIS_RIGHT_X));
        float ry = applyDeadzone(GetGamepadAxisMovement(GAMEPAD_ID, GAMEPAD_AXIS_RIGHT_Y));
        virtualCursor.x += rx * CURSOR_SPEED * dt;
        virtualCursor.y += ry * CURSOR_SPEED * dt;
        if (virtualCursor.x < 0.0f) virtualCursor.x = 0.0f;
        if (virtualCursor.y < 0.0f) virtualCursor.y = 0.0f;
        if (virtualCursor.x > (float)SCREEN_WIDTH) virtualCursor.x = (float)SCREEN_WIDTH;
        if (virtualCursor.y > (float)SCREEN_HEIGHT) virtualCursor.y = (float)SCREEN_HEIGHT;
    }

    (void)dt;
}

bool Input_UsingGamepad(void) {
    return activeDevice == INPUT_DEVICE_GAMEPAD && gamepadConnected();
}

Vector2 Input_GetPointer(void) {
    if (Input_UsingGamepad()) return virtualCursor;
    return GetMousePosition();
}

bool Input_PointerPressed(void) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return true;
    if (gamepadConnected() && IsGamepadButtonPressed(GAMEPAD_ID, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
        return true;
    return false;
}

bool Input_PointerDown(void) {
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) return true;
    if (gamepadConnected() && IsGamepadButtonDown(GAMEPAD_ID, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
        return true;
    return false;
}

Vector2 Input_GetMove(void) {
    Vector2 move = { 0.0f, 0.0f };

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) move.y -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) move.y += 1.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) move.x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) move.x += 1.0f;

    if (gamepadConnected()) {
        float lx = applyDeadzone(GetGamepadAxisMovement(GAMEPAD_ID, GAMEPAD_AXIS_LEFT_X));
        float ly = applyDeadzone(GetGamepadAxisMovement(GAMEPAD_ID, GAMEPAD_AXIS_LEFT_Y));
        move.x += lx;
        move.y += ly;

        if (IsGamepadButtonDown(GAMEPAD_ID, GAMEPAD_BUTTON_LEFT_FACE_UP)) move.y -= 1.0f;
        if (IsGamepadButtonDown(GAMEPAD_ID, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) move.y += 1.0f;
        if (IsGamepadButtonDown(GAMEPAD_ID, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) move.x -= 1.0f;
        if (IsGamepadButtonDown(GAMEPAD_ID, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) move.x += 1.0f;
    }

    float len = sqrtf(move.x * move.x + move.y * move.y);
    if (len > 1.0f) {
        move.x /= len;
        move.y /= len;
    }
    return move;
}

bool Input_PressedConfirm(void) {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) return true;
    if (gamepadConnected() && IsGamepadButtonPressed(GAMEPAD_ID, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
        return true;
    return false;
}

bool Input_PressedCancel(void) {
    if (IsKeyPressed(KEY_ESCAPE)) return true;
    if (gamepadConnected()) {
        if (IsGamepadButtonPressed(GAMEPAD_ID, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) return true;
        if (IsGamepadButtonPressed(GAMEPAD_ID, GAMEPAD_BUTTON_MIDDLE_RIGHT)) return true;
    }
    return false;
}

bool Input_PressedGateTrue(void) {
    if (IsKeyPressed(KEY_V)) return true;
    if (gamepadConnected() && IsGamepadButtonPressed(GAMEPAD_ID, GAMEPAD_BUTTON_LEFT_FACE_UP))
        return true;
    return false;
}

bool Input_PressedGateFalse(void) {
    if (IsKeyPressed(KEY_F)) return true;
    if (gamepadConnected() && IsGamepadButtonPressed(GAMEPAD_ID, GAMEPAD_BUTTON_RIGHT_FACE_LEFT))
        return true;
    return false;
}

void Input_DrawCursor(void) {
    if (!Input_UsingGamepad()) return;

    Vector2 p = virtualCursor;
    Color col = COLOR_NEON_CYAN;
    Color glow = ColorAlpha(col, 0.35f);

    DrawCircleV(p, 14.0f, glow);
    DrawCircleLinesV(p, 10.0f, col);
    DrawLineEx((Vector2){ p.x - 16.0f, p.y }, (Vector2){ p.x - 6.0f, p.y }, 2.0f, col);
    DrawLineEx((Vector2){ p.x + 6.0f, p.y }, (Vector2){ p.x + 16.0f, p.y }, 2.0f, col);
    DrawLineEx((Vector2){ p.x, p.y - 16.0f }, (Vector2){ p.x, p.y - 6.0f }, 2.0f, col);
    DrawLineEx((Vector2){ p.x, p.y + 6.0f }, (Vector2){ p.x, p.y + 16.0f }, 2.0f, col);
}

void Input_NotifyDamage(void) {
    startRumble(RUMBLE_DAMAGE_TIME, RUMBLE_DAMAGE_STR);
}

void Input_NotifyPhaseWin(void) {
    startRumble(RUMBLE_PHASE_TIME, RUMBLE_PHASE_STR);
}

void Input_NotifyComboLife(void) {
    startRumble(RUMBLE_COMBO_TIME, RUMBLE_COMBO_STR);
}
