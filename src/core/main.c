#include "raylib.h"
#include "game.h"
#include "raymath.h"

// Virtual canvas dimensions — all game logic and coordinates use this
#define VIRTUAL_W SCREEN_WIDTH
#define VIRTUAL_H SCREEN_HEIGHT

static int lastWinW = 0, lastWinH = 0;

// Pre-transform mouse so GetMousePosition() returns virtual (1280x720) coords
static void UpdateMouseTransform(void) {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    if (sw == lastWinW && sh == lastWinH) return;
    lastWinW = sw; lastWinH = sh;

    float scaleX = (float)sw / (float)VIRTUAL_W;
    float scaleY = (float)sh / (float)VIRTUAL_H;
    float scale  = (scaleX < scaleY) ? scaleX : scaleY;
    float offX   = (sw - VIRTUAL_W * scale) * 0.5f;
    float offY   = (sh - VIRTUAL_H * scale) * 0.5f;

    SetMouseScale(1.0f / scale, 1.0f / scale);
    SetMouseOffset((int)(-offX / scale), (int)(-offY / scale));
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

        // Build a Camera2D that maps 1280x720 virtual coords → full window
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();
        float scaleX = (float)sw / (float)VIRTUAL_W;
        float scaleY = (float)sh / (float)VIRTUAL_H;
        float scale  = (scaleX < scaleY) ? scaleX : scaleY;
        float offX   = (sw - VIRTUAL_W * scale) * 0.5f;
        float offY   = (sh - VIRTUAL_H * scale) * 0.5f;

        Camera2D cam = {
            .offset   = { offX, offY },
            .target   = { 0.0f, 0.0f },
            .rotation = 0.0f,
            .zoom     = scale
        };

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(cam);
        DrawGame();   // draws at virtual 1280x720 coords; HIGHDPI renders at native res
        EndMode2D();
        EndDrawing();
    }

    UnloadGame();
    CloseWindow();
    return 0;
}
