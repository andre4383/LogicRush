#include "../core/game.h"
#include "../core/screens.h"
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
        InitGameplayScreen();
    }
    
    if (exitHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        exit(0); // Exit game safely
    }
}

void DrawTitleScreen(void) {
    // Beautiful cadet background
    ClearBackground((Color){ 10, 15, 26, 255 }); 
    
    // Draw background grid lines (cyber style)
    for (int x = 0; x < SCREEN_WIDTH; x += 40) {
        Color col = (x % 160 == 0) ? ColorAlpha((Color){ 59, 130, 246, 255 }, 0.08f) : ColorAlpha((Color){ 59, 130, 246, 255 }, 0.03f);
        DrawLine(x, 0, x, SCREEN_HEIGHT, col);
    }
    for (int y = 0; y < SCREEN_HEIGHT; y += 40) {
        Color col = (y % 160 == 0) ? ColorAlpha((Color){ 59, 130, 246, 255 }, 0.08f) : ColorAlpha((Color){ 59, 130, 246, 255 }, 0.03f);
        DrawLine(0, y, SCREEN_WIDTH, y, col);
    }
    
    // Draw high-tech circuit layouts in corners
    
    // Top-Left Tech Corner
    DrawLineEx((Vector2){20, 20}, (Vector2){120, 20}, 2.0f, ColorAlpha((Color){ 59, 130, 246, 255 }, 0.3f));
    DrawLineEx((Vector2){120, 20}, (Vector2){150, 50}, 2.0f, ColorAlpha((Color){ 59, 130, 246, 255 }, 0.3f));
    DrawCircle(20, 20, 4.0f, ColorAlpha((Color){ 59, 130, 246, 255 }, 0.5f));
    DrawCircle(150, 50, 4.0f, ColorAlpha((Color){ 59, 130, 246, 255 }, 0.5f));
    
    // Bottom-Right Tech Corner
    DrawLineEx((Vector2){SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20}, (Vector2){SCREEN_WIDTH - 120, SCREEN_HEIGHT - 20}, 2.0f, ColorAlpha((Color){ 168, 85, 247, 255 }, 0.3f));
    DrawLineEx((Vector2){SCREEN_WIDTH - 120, SCREEN_HEIGHT - 20}, (Vector2){SCREEN_WIDTH - 150, SCREEN_HEIGHT - 50}, 2.0f, ColorAlpha((Color){ 168, 85, 247, 255 }, 0.3f));
    DrawCircle(SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20, 4.0f, ColorAlpha((Color){ 168, 85, 247, 255 }, 0.5f));
    DrawCircle(SCREEN_WIDTH - 150, SCREEN_HEIGHT - 50, 4.0f, ColorAlpha((Color){ 168, 85, 247, 255 }, 0.5f));
    
    // Title with glowing accent
    const char* titleText = "LOGIC RUSH";
    int titleFontSize = 70;
    int titleWidth = MeasureText(titleText, titleFontSize);
    
    // Multiple glow layers
    Color glowColor = ColorAlpha((Color){ 59, 130, 246, 255 }, titleGlow * 0.4f);
    DrawText(titleText, SCREEN_WIDTH / 2 - titleWidth / 2 + 4, SCREEN_HEIGHT / 4 + 4, titleFontSize, glowColor);
    DrawText(titleText, SCREEN_WIDTH / 2 - titleWidth / 2, SCREEN_HEIGHT / 4, titleFontSize, (Color){ 96, 165, 250, 255 }); 
    
    // Subtitle
    const char* subtitleText = "Desafios Lógicos de Labirinto";
    int subtitleFontSize = 20;
    int subtitleWidth = MeasureText(subtitleText, subtitleFontSize);
    DrawText(subtitleText, SCREEN_WIDTH / 2 - subtitleWidth / 2, SCREEN_HEIGHT / 4 + 90, subtitleFontSize, (Color){ 148, 163, 184, 255 }); 
    
    // Draw Buttons
    // Play Button
    Color playGlow = playHover ? (Color){ 34, 197, 94, 255 } : (Color){ 59, 130, 246, 255 }; 
    DrawRectangleRounded(playButton, 0.20f, 4, ColorAlpha((Color){ 15, 23, 42, 255 }, 0.80f));
    DrawRectangleRoundedLines(playButton, 0.20f, 4, playGlow);
    if (playHover) {
        Rectangle glowPlay = { playButton.x - 2, playButton.y - 2, playButton.width + 4, playButton.height + 4 };
        DrawRectangleRoundedLines(glowPlay, 0.20f, 4, ColorAlpha(playGlow, 0.4f));
    }
    
    const char* playText = "INICIAR INTERFACE";
    int playTextWidth = MeasureText(playText, 18);
    DrawText(playText, playButton.x + playButton.width/2 - playTextWidth/2, playButton.y + playButton.height/2 - 9, 18, playGlow);
    
    // Exit Button
    Color exitGlow = exitHover ? (Color){ 239, 68, 68, 255 } : (Color){ 71, 85, 105, 255 }; 
    DrawRectangleRounded(exitButton, 0.20f, 4, ColorAlpha((Color){ 15, 23, 42, 255 }, 0.80f));
    DrawRectangleRoundedLines(exitButton, 0.20f, 4, exitGlow);
    if (exitHover) {
        Rectangle glowExit = { exitButton.x - 2, exitButton.y - 2, exitButton.width + 4, exitButton.height + 4 };
        DrawRectangleRoundedLines(glowExit, 0.20f, 4, ColorAlpha(exitGlow, 0.4f));
    }
    
    const char* exitText = "FECHAR SISTEMA";
    int exitTextWidth = MeasureText(exitText, 18);
    DrawText(exitText, exitButton.x + exitButton.width/2 - exitTextWidth/2, exitButton.y + exitButton.height/2 - 9, 18, exitGlow);
    
    // Footer
    const char* footerText = "Use o mouse para interagir | Navegação WASD + E no jogo";
    int footerWidth = MeasureText(footerText, 14);
    DrawText(footerText, SCREEN_WIDTH / 2 - footerWidth / 2, SCREEN_HEIGHT - 60, 14, (Color){ 71, 85, 105, 255 });
}

void UnloadTitleScreen(void) {
    // Clean up if any
}
