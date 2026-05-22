#include "../core/game.h"
#include "../core/screens.h"
#include "../core/theme.h"
#include <stdlib.h>

// ── Botões do menu ──────────────────────────────────────────────────────────────
static Rectangle playButton = {
    SCREEN_WIDTH / 2.0f - 160, SCREEN_HEIGHT / 2.0f - 10, 320, 58
};
static Rectangle bossButton = {
    SCREEN_WIDTH / 2.0f - 160, SCREEN_HEIGHT / 2.0f + 68, 320, 58
};
static Rectangle exitButton = {
    SCREEN_WIDTH / 2.0f - 160, SCREEN_HEIGHT / 2.0f + 146, 320, 58
};

static bool playHover = false;
static bool bossHover = false;
static bool exitHover = false;
static float titleGlow = 0.0f;
static int   titleGlowDir = 1;

void InitTitleScreen(void) {
    playHover     = false;
    bossHover     = false;
    exitHover     = false;
    titleGlow     = 0.0f;
    titleGlowDir  = 1;
}

void UpdateTitleScreen(void) {
    Vector2 mouse = GetMousePosition();

    playHover = CheckCollisionPointRec(mouse, playButton);
    bossHover = CheckCollisionPointRec(mouse, bossButton);
    exitHover = CheckCollisionPointRec(mouse, exitButton);

    // Anima brilho do título
    titleGlow += 0.018f * titleGlowDir;
    if (titleGlow >= 1.0f) { titleGlow = 1.0f; titleGlowDir = -1; }
    else if (titleGlow <= 0.0f) { titleGlow = 0.0f; titleGlowDir = 1; }

    if (playHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        InitGameplayScreen();
        currentScreen = SCREEN_GAMEPLAY;
    }

    if (bossHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        InitBossScreen();   // Reseta estado ao entrar na boss fight
        currentScreen = SCREEN_BOSS;
    }

    if (exitHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        exit(0);
}

void DrawTitleScreen(void) {
    float t = (float)GetTime();

    // 1. Fundo e grade
    ClearBackground(COLOR_BG_DARK);
    DrawThemeGrid(SCREEN_WIDTH, SCREEN_HEIGHT, CELL_SIZE);

    // 2. Cantos decorativos (circuito)
    // Superior esquerdo
    DrawLineEx((Vector2){20, 20}, (Vector2){120, 20}, 2.0f,
               ColorAlpha(COLOR_GRID_LINE, 0.3f));
    DrawLineEx((Vector2){120, 20}, (Vector2){150, 50}, 2.0f,
               ColorAlpha(COLOR_GRID_LINE, 0.3f));
    DrawCircle(20,  20,  4.0f, ColorAlpha(COLOR_GRID_LINE,  0.5f));
    DrawCircle(150, 50,  4.0f, ColorAlpha(COLOR_GRID_LINE,  0.5f));

    // Inferior direito
    DrawLineEx((Vector2){SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20},
               (Vector2){SCREEN_WIDTH - 120, SCREEN_HEIGHT - 20},
               2.0f, ColorAlpha(COLOR_NEON_PURPLE, 0.3f));
    DrawLineEx((Vector2){SCREEN_WIDTH - 120, SCREEN_HEIGHT - 20},
               (Vector2){SCREEN_WIDTH - 150, SCREEN_HEIGHT - 50},
               2.0f, ColorAlpha(COLOR_NEON_PURPLE, 0.3f));
    DrawCircle(SCREEN_WIDTH - 20,  SCREEN_HEIGHT - 20, 4.0f,
               ColorAlpha(COLOR_NEON_PURPLE, 0.5f));
    DrawCircle(SCREEN_WIDTH - 150, SCREEN_HEIGHT - 50, 4.0f,
               ColorAlpha(COLOR_NEON_PURPLE, 0.5f));

    // 3. Título com glow
    const char *titleText    = "LOGIC RUSH";
    int         titleFontSize = 72;
    int         titleW        = MeasureText(titleText, titleFontSize);

    Color glowColor = ColorAlpha(COLOR_GRID_LINE, titleGlow * 0.45f);
    DrawText(titleText, SCREEN_WIDTH / 2 - titleW / 2 + 4,
             SCREEN_HEIGHT / 4 + 4, titleFontSize, glowColor);
    DrawText(titleText, SCREEN_WIDTH / 2 - titleW / 2,
             SCREEN_HEIGHT / 4, titleFontSize, COLOR_TEXT_CYBER);

    // Subtítulo
    const char *sub  = "Desafios de Logica Proposicional";
    int         subW = MeasureText(sub, 20);
    DrawText(sub, SCREEN_WIDTH / 2 - subW / 2,
             SCREEN_HEIGHT / 4 + 92, 20, COLOR_TEXT_MUTED);

    // Linha separadora animada
    float lineW = 200.0f + sinf(t * 2.0f) * 40.0f;
    DrawLineEx(
        (Vector2){SCREEN_WIDTH / 2.0f - lineW / 2, SCREEN_HEIGHT / 4.0f + 125},
        (Vector2){SCREEN_WIDTH / 2.0f + lineW / 2, SCREEN_HEIGHT / 4.0f + 125},
        1.5f, ColorAlpha(COLOR_NEON_CYAN, 0.35f));

    // 4. Vignette
    DrawThemeVignette(SCREEN_WIDTH, SCREEN_HEIGHT);

    // 5. Botões
    DrawThemeButton(playButton, "FASE 2: LABIRINTO",   18, playHover, COLOR_NEON_CYAN);
    DrawThemeButton(bossButton, "FASE 3: BOSS FIGHT",  18, bossHover, COLOR_NEON_PURPLE);
    DrawThemeButton(exitButton, "FECHAR SISTEMA",       18, exitHover, COLOR_NEON_RED);

    // Labels dos botões
    int lw1 = MeasureText("Labirinto de Cabos", 13);
    DrawText("Labirinto de Cabos",
             (int)(playButton.x + playButton.width / 2 - lw1 / 2),
             (int)(playButton.y + playButton.height + 4), 13, COLOR_TEXT_MUTED);

    int lw2 = MeasureText("Meeting Equal — Logica Proposicional", 13);
    DrawText("Meeting Equal — Logica Proposicional",
             (int)(bossButton.x + bossButton.width / 2 - lw2 / 2),
             (int)(bossButton.y + bossButton.height + 4), 13, COLOR_TEXT_MUTED);

    // Rodapé
    const char *footer = "Use o mouse para navegar entre as fases";
    int         fw     = MeasureText(footer, 14);
    DrawText(footer, SCREEN_WIDTH / 2 - fw / 2,
             SCREEN_HEIGHT - 32, 14, COLOR_TEXT_MUTED);
}

void UnloadTitleScreen(void) {
    // Sem recursos para liberar
}
