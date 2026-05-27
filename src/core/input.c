#include "input.h"
#include "game.h"
#include "theme.h"
#include "raymath.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define MAX_GAMEPAD_SLOTS       4
#define STICK_DEADZONE          0.20f
#define FOCUS_REPEAT_INITIAL    0.20f
#define FOCUS_REPEAT_RATE       0.08f
#define RUMBLE_DAMAGE_TIME      0.35f
#define RUMBLE_DAMAGE_STR       0.90f
#define RUMBLE_PHASE_TIME       0.50f
#define RUMBLE_PHASE_STR        0.65f
#define RUMBLE_COMBO_TIME       0.20f
#define RUMBLE_COMBO_STR        0.45f

#if defined(__APPLE__)
#define MAPPING_PLATFORM "platform:Mac OS X"
#elif defined(_WIN32)
#define MAPPING_PLATFORM "platform:Windows"
#else
#define MAPPING_PLATFORM "platform:Linux"
#endif

typedef enum {
    INPUT_DEVICE_MOUSE = 0,
    INPUT_DEVICE_GAMEPAD
} InputDevice;

static InputDevice activeDevice = INPUT_DEVICE_MOUSE;
static Vector2 lastMousePos = { 0.0f, 0.0f };
static int activeGamepad = -1;
static bool wasGamepadConnected = false;
static bool inputDebug = false;

static float focusRepeatTimer = 0.0f;
static int focusNavHeld = 0;

static int gamepadId(void) {
    return activeGamepad;
}

static bool gamepadConnected(void) {
    return activeGamepad >= 0 && IsGamepadAvailable(activeGamepad);
}

static bool mappingLineForPlatform(const char *line) {
    return strstr(line, MAPPING_PLATFORM) != NULL;
}

static void loadGamepadMappingsFromText(const char *text) {
    const char *line = text;
    while (line && *line) {
        const char *eol = strchr(line, '\n');
        size_t len = eol ? (size_t)(eol - line) : strlen(line);
        while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n')) len--;

        if (len > 0 && line[0] != '#') {
            char buf[512];
            if (len >= sizeof(buf)) len = sizeof(buf) - 1;
            memcpy(buf, line, len);
            buf[len] = '\0';
            if (mappingLineForPlatform(buf)) (void)SetGamepadMappings(buf);
        }

        if (!eol) break;
        line = eol + 1;
    }
}

static void loadGamepadMappings(void) {
    static const char *paths[] = {
        "assets/gamecontrollerdb.txt",
        "./gamecontrollerdb.txt",
        NULL
    };

    for (int i = 0; paths[i] != NULL; i++) {
        if (!FileExists(paths[i])) continue;
        char *text = LoadFileText(paths[i]);
        if (text == NULL) continue;
        loadGamepadMappingsFromText(text);
        UnloadFileText(text);
        return;
    }
}

static void scanActiveGamepad(void) {
    activeGamepad = -1;
    for (int i = 0; i < MAX_GAMEPAD_SLOTS; i++) {
        if (IsGamepadAvailable(i)) {
            activeGamepad = i;
            return;
        }
    }
}

static float applyDeadzone(float v) {
    if (fabsf(v) < STICK_DEADZONE) return 0.0f;
    float sign = (v > 0.0f) ? 1.0f : -1.0f;
    float norm = (fabsf(v) - STICK_DEADZONE) / (1.0f - STICK_DEADZONE);
    return sign * norm;
}

static bool gamepadAnyActivity(void) {
    if (!gamepadConnected()) return false;
    int id = gamepadId();

    if (fabsf(GetGamepadAxisMovement(id, GAMEPAD_AXIS_LEFT_X)) > STICK_DEADZONE) return true;
    if (fabsf(GetGamepadAxisMovement(id, GAMEPAD_AXIS_LEFT_Y)) > STICK_DEADZONE) return true;
    if (fabsf(GetGamepadAxisMovement(id, GAMEPAD_AXIS_RIGHT_X)) > STICK_DEADZONE) return true;
    if (fabsf(GetGamepadAxisMovement(id, GAMEPAD_AXIS_RIGHT_Y)) > STICK_DEADZONE) return true;

    for (int b = 0; b <= GAMEPAD_BUTTON_RIGHT_THUMB; b++) {
        if (IsGamepadButtonPressed(id, b) || IsGamepadButtonDown(id, b)) return true;
    }
    return false;
}

static void startRumble(float duration, float strength) {
    if (!gamepadConnected()) return;
    SetGamepadVibration(gamepadId(), strength, strength, duration);
}

static void clampFocus(int *focus, int count) {
    if (count <= 0) {
        *focus = 0;
        return;
    }
    if (*focus < 0) *focus = 0;
    if (*focus >= count) *focus = count - 1;
}

static int moveFocusIndex(int focus, int count, int cols, int dr, int dc,
                          InputFocusSkipFn skip, void *user) {
    if (count <= 0) return 0;

    int start = focus;
    for (int attempt = 0; attempt < count; attempt++) {
        if (cols <= 1) {
            focus += dr;
        } else if (cols >= count) {
            focus += dc;
        } else {
            int row = focus / cols;
            int col = focus % cols;
            row += dr;
            col += dc;
            int rows = (count + cols - 1) / cols;
            if (row < 0) row = rows - 1;
            if (row >= rows) row = 0;
            if (col < 0) col = cols - 1;
            if (col >= cols) col = 0;
            focus = row * cols + col;
            if (focus >= count) focus = count - 1;
        }

        if (focus < 0) focus = count - 1;
        if (focus >= count) focus = 0;

        if (!skip || !skip(focus, user)) return focus;
    }
    return start;
}

static int readFocusNav(int *outDr, int *outDc) {
    *outDr = 0;
    *outDc = 0;

    if (!gamepadConnected()) return 0;

    int id = gamepadId();
    int nav = 0;

    if (IsGamepadButtonPressed(id, GAMEPAD_BUTTON_LEFT_FACE_UP)) *outDr = -1;
    if (IsGamepadButtonPressed(id, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) *outDr = 1;
    if (IsGamepadButtonPressed(id, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) *outDc = -1;
    if (IsGamepadButtonPressed(id, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) *outDc = 1;

    float lx = applyDeadzone(GetGamepadAxisMovement(id, GAMEPAD_AXIS_LEFT_X));
    float ly = applyDeadzone(GetGamepadAxisMovement(id, GAMEPAD_AXIS_LEFT_Y));
    if (ly < -0.5f) *outDr = -1;
    if (ly > 0.5f) *outDr = 1;
    if (lx < -0.5f) *outDc = -1;
    if (lx > 0.5f) *outDc = 1;

    if (*outDr != 0 || *outDc != 0) nav = 1;

    if (nav == 0) return 0;

    int held = (*outDr + 2) * 3 + (*outDc + 2);
    float dt = GetFrameTime();

    if (held != focusNavHeld) {
        focusNavHeld = held;
        focusRepeatTimer = FOCUS_REPEAT_INITIAL;
        return 1;
    }

    focusRepeatTimer -= dt;
    if (focusRepeatTimer <= 0.0f) {
        focusRepeatTimer = FOCUS_REPEAT_RATE;
        return 1;
    }
    return 0;
}

void Input_Init(void) {
    const char *dbg = getenv("LOGICRUSH_INPUT_DEBUG");
    inputDebug = (dbg != NULL && dbg[0] == '1');

    loadGamepadMappings();
    scanActiveGamepad();

    lastMousePos = GetMousePosition();
    activeDevice = INPUT_DEVICE_MOUSE;
    wasGamepadConnected = gamepadConnected();
    focusNavHeld = 0;
    focusRepeatTimer = 0.0f;
}

void Input_Update(float dt) {
    (void)dt;
    if (IsKeyPressed(KEY_F12)) inputDebug = !inputDebug;

    scanActiveGamepad();
    bool connectedNow = gamepadConnected();

    if (connectedNow && !wasGamepadConnected) {
        loadGamepadMappings();
        scanActiveGamepad();
    }
    wasGamepadConnected = gamepadConnected();

    Vector2 mouse = GetMousePosition();
    if (Vector2Distance(mouse, lastMousePos) > 1.0f) {
        activeDevice = INPUT_DEVICE_MOUSE;
    }
    lastMousePos = mouse;

    if (gamepadAnyActivity()) {
        activeDevice = INPUT_DEVICE_GAMEPAD;
    }
}

bool Input_IsConnected(void) {
    return gamepadConnected();
}

const char *Input_GetGamepadName(void) {
    if (!gamepadConnected()) return "";
    return GetGamepadName(gamepadId());
}

bool Input_UsingGamepad(void) {
    return activeDevice == INPUT_DEVICE_GAMEPAD && gamepadConnected();
}

void Input_FocusReset(int *focus, int count) {
    (void)count;
    *focus = 0;
    focusNavHeld = 0;
    focusRepeatTimer = 0.0f;
}

void Input_FocusUpdateSkip(int *focus, int count, int cols,
                           InputFocusSkipFn skip, void *user) {
    if (count <= 0) return;
    clampFocus(focus, count);
    if (!Input_UsingGamepad()) return;

    if (cols < 1) cols = 1;

    int dr = 0, dc = 0;
    if (!readFocusNav(&dr, &dc)) return;

    if (cols == 1) dc = 0;
    if (cols >= count) dr = 0;

    if (dr == 0 && dc == 0) return;

    *focus = moveFocusIndex(*focus, count, cols, dr, dc, skip, user);
}

void Input_FocusUpdate(int *focus, int count, int cols) {
    Input_FocusUpdateSkip(focus, count, cols, NULL, NULL);
}

bool Input_ItemHot(int index, Rectangle rect, int *focus, int count, int cols) {
    (void)cols;
    if (Input_UsingGamepad()) {
        clampFocus(focus, count);
        return index == *focus;
    }
    return CheckCollisionPointRec(GetMousePosition(), rect);
}

bool Input_ItemPressed(int index, Rectangle rect, int *focus, int count, int cols) {
    (void)cols;
    if (Input_UsingGamepad()) {
        clampFocus(focus, count);
        return index == *focus && Input_PressedConfirm();
    }
    return CheckCollisionPointRec(GetMousePosition(), rect) && Input_PointerPressed();
}

Vector2 Input_GetPointer(void) {
    return GetMousePosition();
}

bool Input_PointerPressed(void) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return true;
    if (gamepadConnected() &&
        IsGamepadButtonPressed(gamepadId(), GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
        return true;
    return false;
}

bool Input_PointerDown(void) {
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) return true;
    if (gamepadConnected() &&
        IsGamepadButtonDown(gamepadId(), GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
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
        int id = gamepadId();
        float lx = applyDeadzone(GetGamepadAxisMovement(id, GAMEPAD_AXIS_LEFT_X));
        float ly = applyDeadzone(GetGamepadAxisMovement(id, GAMEPAD_AXIS_LEFT_Y));
        move.x += lx;
        move.y += ly;

        if (IsGamepadButtonDown(id, GAMEPAD_BUTTON_LEFT_FACE_UP)) move.y -= 1.0f;
        if (IsGamepadButtonDown(id, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) move.y += 1.0f;
        if (IsGamepadButtonDown(id, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) move.x -= 1.0f;
        if (IsGamepadButtonDown(id, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) move.x += 1.0f;
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
    if (gamepadConnected() &&
        IsGamepadButtonPressed(gamepadId(), GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
        return true;
    return false;
}

bool Input_PressedCancel(void) {
    if (IsKeyPressed(KEY_ESCAPE)) return true;
    if (gamepadConnected()) {
        int id = gamepadId();
        if (IsGamepadButtonPressed(id, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) return true;
        if (IsGamepadButtonPressed(id, GAMEPAD_BUTTON_MIDDLE_RIGHT)) return true;
    }
    return false;
}

bool Input_PressedGateTrue(void) {
    if (IsKeyPressed(KEY_V)) return true;
    if (gamepadConnected() &&
        IsGamepadButtonPressed(gamepadId(), GAMEPAD_BUTTON_LEFT_FACE_UP))
        return true;
    return false;
}

bool Input_PressedGateFalse(void) {
    if (IsKeyPressed(KEY_F)) return true;
    if (gamepadConnected() &&
        IsGamepadButtonPressed(gamepadId(), GAMEPAD_BUTTON_RIGHT_FACE_LEFT))
        return true;
    return false;
}

void Input_DrawCursor(void) {
}

void Input_DrawDebug(void) {
    if (!inputDebug) return;

    int y = 8;
    const int lh = 18;
    DrawRectangle(4, 4, 420, 110, ColorAlpha(BLACK, 0.75f));

    DrawText(TextFormat("Input debug (F12 toggle)"), 10, y, 14, COLOR_NEON_CYAN);
    y += lh;

    for (int i = 0; i < MAX_GAMEPAD_SLOTS; i++) {
        DrawText(TextFormat("pad[%d]: %s", i, IsGamepadAvailable(i) ? "yes" : "no"), 10, y, 14,
                 IsGamepadAvailable(i) ? COLOR_NEON_GREEN : COLOR_TEXT_MUTED);
        y += lh;
    }

    DrawText(TextFormat("active: %d  name: %s", activeGamepad, Input_GetGamepadName()), 10, y, 14,
             COLOR_TEXT_MAIN);
    y += lh;

    DrawText(TextFormat("device: %s", Input_UsingGamepad() ? "gamepad" : "mouse"), 10, y, 14,
             COLOR_TEXT_MAIN);
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
