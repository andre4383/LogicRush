#include "raylib.h"
#include "../core/game.h"
#include "../core/theme.h"

// Player Base Structure
static Vector2 playerPos;
static float playerRadius = 14.0f;
static float playerSpeed = 250.0f;

void InitGameplayScreen(void) {
    // Start player in the center of the screen
    playerPos = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
}

void UpdateGameplayScreen(void) {
    // ESC to return to menu
    if (IsKeyPressed(KEY_ESCAPE)) {
        currentScreen = SCREEN_TITLE;
    }
    
    // Player movement
    float dt = GetFrameTime();
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) playerPos.y -= playerSpeed * dt;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) playerPos.y += playerSpeed * dt;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) playerPos.x -= playerSpeed * dt;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) playerPos.x += playerSpeed * dt;
    
    // Keep player inside screen boundaries
    if (playerPos.x < playerRadius) playerPos.x = playerRadius;
    if (playerPos.x > SCREEN_WIDTH - playerRadius) playerPos.x = SCREEN_WIDTH - playerRadius;
    if (playerPos.y < playerRadius + CELL_SIZE) playerPos.y = playerRadius + CELL_SIZE; // Avoid HUD
    if (playerPos.y > SCREEN_HEIGHT - playerRadius) playerPos.y = SCREEN_HEIGHT - playerRadius;
}

void DrawGameplayScreen(void) {
    // 1. Clear background to standard dark theme
    ClearBackground(COLOR_BG_DARK);
    
    // 2. Draw standard background cyber grid
    DrawThemeGrid(SCREEN_WIDTH, SCREEN_HEIGHT, CELL_SIZE);
    
    // 3. Draw game components here (e.g. Player)
    // Draw player glow aura
    float timeVal = (float)GetTime();
    float auraRadius = playerRadius + 4.0f + sinf(timeVal * 12.0f) * 2.0f;
    DrawCircleV(playerPos, auraRadius, ColorAlpha(COLOR_NEON_GREEN, 0.20f));
    // Draw player core
    DrawCircleV(playerPos, playerRadius, COLOR_NEON_GREEN);
    DrawCircleV(playerPos, playerRadius - 4.0f, (Color){ 187, 247, 208, 255 });
    
    // 4. Draw vignette for cinematic feel
    DrawThemeVignette(SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // 5. Draw floating glassmorphism HUD panel
    Rectangle hudRect = { 15, 10, SCREEN_WIDTH - 30, CELL_SIZE };
    DrawThemeGlassPanel(hudRect, 0.20f, ColorAlpha(COLOR_GRID_LINE, 0.25f));
    
    // Text inside HUD
    DrawText("LOGIC RUSH: TELA DE GAMEPLAY (BASE)", 35, 20, 20, COLOR_TEXT_CYBER);
    DrawText("ESC: Voltar ao Menu | WASD / Setas: Mover", SCREEN_WIDTH - 420, 23, 14, COLOR_TEXT_MUTED);
}

void UnloadGameplayScreen(void) {
    // Clean up resources here if needed
}
