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

    SetMouseScale(1.0f / scaleX, 1.0f / scaleY);
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

    RenderTexture2D canvas = LoadRenderTexture(VIRTUAL_W, VIRTUAL_H);
    SetTextureFilter(canvas.texture, TEXTURE_FILTER_BILINEAR);

    while (!WindowShouldClose()) {
        UpdateMouseTransform();
        UpdateGame();

        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        BeginTextureMode(canvas);
        ClearBackground((Color){10, 12, 28, 255});
        DrawGame();
        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(
            canvas.texture,
            (Rectangle){0.0f, 0.0f, (float)VIRTUAL_W, -(float)VIRTUAL_H},
            (Rectangle){0.0f, 0.0f, (float)sw, (float)sh},
            (Vector2){0.0f, 0.0f},
            0.0f,
            WHITE
        );
        EndDrawing();
    }

    UnloadRenderTexture(canvas);
    UnloadGame();
    CloseWindow();
    return 0;
}
