#include "raylib.h"
#include "rlgl.h"
#include "game.h"
#include "raymath.h"

// Virtual canvas dimensions — all game logic and coordinates use this
#define VIRTUAL_W SCREEN_WIDTH
#define VIRTUAL_H SCREEN_HEIGHT

static int lastWinW = 0, lastWinH = 0;

// Pre-transform mouse so GetMousePosition() returns virtual (1280x720) coords.
// Uses logical pixels (GetScreenWidth) — correct because Raylib reports mouse
// in logical coords regardless of HiDPI.
static void UpdateMouseTransform(void) {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    if (sw == lastWinW && sh == lastWinH) return;
    lastWinW = sw; lastWinH = sh;

    SetMouseScale((float)VIRTUAL_W / (float)sw, (float)VIRTUAL_H / (float)sh);
    SetMouseOffset(0, 0);
}

int main(void) {
    // FLAG_WINDOW_HIGHDPI: framebuffer uses native Retina/HiDPI resolution on macOS
    // — rendering happens at full physical pixel density, eliminating blur from upscaling
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(VIRTUAL_W, VIRTUAL_H, "Logic Rush - Desafios de Logica Proposicional");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    // Start fullscreen at monitor native resolution
    int mon = GetCurrentMonitor();
    SetWindowSize(GetMonitorWidth(mon), GetMonitorHeight(mon));
    ToggleFullscreen();

    InitGame();

    while (!WindowShouldClose()) {
        UpdateMouseTransform();
        UpdateGame();

        // GetRenderWidth/Height returns physical framebuffer pixels (HiDPI-aware).
        // rlScalef maps virtual 1280x720 coords to fill the entire physical framebuffer
        // with independent X/Y scale — no black bars, no texture downscale blur.
        float scaleX = (float)GetRenderWidth()  / (float)VIRTUAL_W;
        float scaleY = (float)GetRenderHeight() / (float)VIRTUAL_H;

        BeginDrawing();
        ClearBackground(BLACK);
        rlPushMatrix();
        rlScalef(scaleX, scaleY, 1.0f);
        DrawGame();
        rlPopMatrix();
        EndDrawing();
    }

    UnloadGame();
    CloseWindow();
    return 0;
}
