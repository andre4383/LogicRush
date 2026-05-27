#include "raylib.h"
#include "game.h"
#include "raymath.h"

// Virtual canvas dimensions (game logic always runs at this resolution)
#define VIRTUAL_W SCREEN_WIDTH
#define VIRTUAL_H SCREEN_HEIGHT

static RenderTexture2D virtualTarget;
static int lastWinW = 0, lastWinH = 0;

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

static void DrawFinalToWindow(void) {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    float scaleX = (float)sw / (float)VIRTUAL_W;
    float scaleY = (float)sh / (float)VIRTUAL_H;
    float scale  = (scaleX < scaleY) ? scaleX : scaleY;
    float offX   = (sw - VIRTUAL_W * scale) * 0.5f;
    float offY   = (sh - VIRTUAL_H * scale) * 0.5f;

    BeginDrawing();
    ClearBackground(BLACK);

    // Note: render texture is flipped vertically — use negative height in src rect
    Rectangle src = { 0, 0, (float)VIRTUAL_W, -(float)VIRTUAL_H };
    Rectangle dst = { offX, offY, VIRTUAL_W * scale, VIRTUAL_H * scale };
    DrawTexturePro(virtualTarget.texture, src, dst, (Vector2){0,0}, 0.0f, WHITE);

    EndDrawing();
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(VIRTUAL_W, VIRTUAL_H, "Logic Rush - Desafios de Logica Proposicional");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    // Create virtual canvas
    virtualTarget = LoadRenderTexture(VIRTUAL_W, VIRTUAL_H);
    SetTextureFilter(virtualTarget.texture, TEXTURE_FILTER_BILINEAR);

    // Start fullscreen
    int mon = GetCurrentMonitor();
    SetWindowSize(GetMonitorWidth(mon), GetMonitorHeight(mon));
    ToggleFullscreen();

    InitGame();

    while (!WindowShouldClose()) {
        UpdateMouseTransform();
        UpdateGame();

        // Render game at virtual resolution
        BeginTextureMode(virtualTarget);
        DrawGame();   // already calls Clear inside DrawXScreen()
        EndTextureMode();

        // Blit scaled to window
        DrawFinalToWindow();
    }

    UnloadRenderTexture(virtualTarget);
    UnloadGame();
    CloseWindow();
    return 0;
}
