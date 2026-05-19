#include "../core/game.h"
#include "../core/screens.h"
#include "../core/theme.h"
#include <stdlib.h>

static Rectangle playButton = { SCREEN_WIDTH / 2.0f - 150, SCREEN_HEIGHT / 2.0f, 300, 60 };
static Rectangle exitButton = { SCREEN_WIDTH / 2.0f - 150, SCREEN_HEIGHT / 2.0f + 80, 300, 60 };

static bool playHover = false;
static bool exitHover = false;
static float titleGlow = 0.0f;
static int titleGlowDirection = 1;

void InitTitleScreen(void) {
    playHover = false;
    exitHover = false;
    titleGlow = 0.0f;
    titleGlowDirection = 1;
}

void UpdateTitleScreen(void) {
    Vector2 mousePos = GetMousePosition();
    
    playHover = CheckCollisionPointRec(mousePos, playButton);
    exitHover = CheckCollisionPointRec(mousePos, exitButton);
    
    // Animate title glow
    titleGlow += 0.02f * titleGlowDirection;
    if (titleGlow >= 1.0f) {
        titleGlow = 1.0f;
        titleGlowDirection = -1;
    } else if (titleGlow <= 0.0f) {
        titleGlow = 0.0f;
        titleGlowDirection = 1;
    }
    
    if (playHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        currentScreen = SCREEN_GAMEPLAY;
    }
    
    if (exitHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        exit(0); // Exit game safely
    }
}

void DrawTitleScreen(void) {
    // 1. Draw standard background & grid
    ClearBackground(COLOR_BG_DARK); 
    DrawThemeGrid(SCREEN_WIDTH, SCREEN_HEIGHT, CELL_SIZE);
    
    // 2. Draw high-tech circuit layouts in corners
    // Top-Left Tech Corner
    DrawLineEx((Vector2){20, 20}, (Vector2){120, 20}, 2.0f, ColorAlpha(COLOR_GRID_LINE, 0.3f));
    DrawLineEx((Vector2){120, 20}, (Vector2){150, 50}, 2.0f, ColorAlpha(COLOR_GRID_LINE, 0.3f));
    DrawCircle(20, 20, 4.0f, ColorAlpha(COLOR_GRID_LINE, 0.5f));
    DrawCircle(150, 50, 4.0f, ColorAlpha(COLOR_GRID_LINE, 0.5f));
    
    // Bottom-Right Tech Corner
    DrawLineEx((Vector2){SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20}, (Vector2){SCREEN_WIDTH - 120, SCREEN_HEIGHT - 20}, 2.0f, ColorAlpha(COLOR_NEON_PURPLE, 0.3f));
    DrawLineEx((Vector2){SCREEN_WIDTH - 120, SCREEN_HEIGHT - 20}, (Vector2){SCREEN_WIDTH - 150, SCREEN_HEIGHT - 50}, 2.0f, ColorAlpha(COLOR_NEON_PURPLE, 0.3f));
    DrawCircle(SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20, 4.0f, ColorAlpha(COLOR_NEON_PURPLE, 0.5f));
    DrawCircle(SCREEN_WIDTH - 150, SCREEN_HEIGHT - 50, 4.0f, ColorAlpha(COLOR_NEON_PURPLE, 0.5f));
    
    // 3. Draw Title with glowing accent
    const char* titleText = "LOGIC RUSH";
    int titleFontSize = 70;
    int titleWidth = MeasureText(titleText, titleFontSize);
    
    Color glowColor = ColorAlpha(COLOR_GRID_LINE, titleGlow * 0.4f);
    DrawText(titleText, SCREEN_WIDTH / 2 - titleWidth / 2 + 4, SCREEN_HEIGHT / 4 + 4, titleFontSize, glowColor);
    DrawText(titleText, SCREEN_WIDTH / 2 - titleWidth / 2, SCREEN_HEIGHT / 4, titleFontSize, COLOR_TEXT_CYBER); 
    
    // Subtitle
    const char* subtitleText = "Desafios Lógicos de Labirinto";
    int subtitleFontSize = 20;
    int subtitleWidth = MeasureText(subtitleText, subtitleFontSize);
    DrawText(subtitleText, SCREEN_WIDTH / 2 - subtitleWidth / 2, SCREEN_HEIGHT / 4 + 90, subtitleFontSize, COLOR_TEXT_MUTED); 
    
    // 4. Draw Vignette effect
    DrawThemeVignette(SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // 5. Draw Buttons using theme helpers
    DrawThemeButton(playButton, "INICIAR INTERFACE", 18, playHover, COLOR_NEON_GREEN);
    DrawThemeButton(exitButton, "FECHAR SISTEMA", 18, exitHover, COLOR_NEON_RED);
    
    // Footer
    const char* footerText = "Use o mouse para interagir | Navegação WASD + E no jogo";
    int footerWidth = MeasureText(footerText, 14);
    DrawText(footerText, SCREEN_WIDTH / 2 - footerWidth / 2, SCREEN_HEIGHT - 60, 14, COLOR_TEXT_MUTED);
}

void UnloadTitleScreen(void) {
    // Clean up if any
}
