#include "../core/game.h"
#include "../core/ranking.h"
#include "../core/screens.h"
#include "../core/theme.h"
#include <stdlib.h>

// ── Botões do menu principal ─────────────────────────────────────────────────
// Modo História
static Rectangle storyButton = {
    SCREEN_WIDTH / 2.0f - 160, SCREEN_HEIGHT / 2.0f + 0, 320, 58
};
// Modo Competitivo
static Rectangle competButton = {
    SCREEN_WIDTH / 2.0f - 160, SCREEN_HEIGHT / 2.0f + 72, 320, 58
};
// Sair
static Rectangle exitButton = {
    SCREEN_WIDTH / 2.0f - 160, SCREEN_HEIGHT / 2.0f + 150, 320, 58
};

static bool storyHover  = false;
static bool competHover = false;
static bool exitHover   = false;
static float titleGlow    = 0.0f;
static int   titleGlowDir = 1;

void InitTitleScreen(void) {
    storyHover   = false;
    competHover  = false;
    exitHover    = false;
    titleGlow    = 0.0f;
    titleGlowDir = 1;
}

void UpdateTitleScreen(void) {
    Vector2 mouse = GetMousePosition();

    storyHover  = CheckCollisionPointRec(mouse, storyButton);
    competHover = CheckCollisionPointRec(mouse, competButton);
    exitHover   = CheckCollisionPointRec(mouse, exitButton);

    // Anima brilho do título
    titleGlow += 0.018f * titleGlowDir;
    if      (titleGlow >= 1.0f) { titleGlow = 1.0f; titleGlowDir = -1; }
    else if (titleGlow <= 0.0f) { titleGlow = 0.0f; titleGlowDir =  1; }

    // ── Botão MODO HISTÓRIA ──────────────────────────────────────────────────
    if (storyHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        globalTimer = 0.0f;
        globalScore = 0;
        globalLives = 3;
        competitiveLoop = 0;
        scoreMultiplier = 1;
        gameRunning = true;
        gamePaused  = false;
        currentGameMode = MODE_STORY;
        Story_ResetFlags();
        InitIntroScreen();
        currentScreen = SCREEN_INTRO;
    }

    // ── Botão MODO COMPETITIVO ───────────────────────────────────────────────
    if (competHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        globalTimer = 0.0f;
        globalScore = 0;
        globalLives = 3;
        competitiveLoop = 0;
        scoreMultiplier = 1;
        gameRunning = true;
        gamePaused  = false;
        currentGameMode = MODE_COMPETITIVE;
        Story_ResetFlags();
        InitQuizScreen();
        currentScreen = SCREEN_QUIZ;
        StartPhaseBanner("FASE 1", "QUIZ DE EQUIVALÊNCIAS");
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
    DrawLineEx((Vector2){20, 20}, (Vector2){120, 20}, 2.0f,
               ColorAlpha(COLOR_GRID_LINE, 0.3f));
    DrawLineEx((Vector2){120, 20}, (Vector2){150, 50}, 2.0f,
               ColorAlpha(COLOR_GRID_LINE, 0.3f));
    DrawCircle(20,  20,  4.0f, ColorAlpha(COLOR_GRID_LINE,  0.5f));
    DrawCircle(150, 50,  4.0f, ColorAlpha(COLOR_GRID_LINE,  0.5f));

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

    // 3. Título
    int   titleFontSize = 90;
    float spacing       = (float)titleFontSize / 10.0f;
    Vector2 szLogic = MeasureTextEx(GetFontDefault(), "LOGIC", (float)titleFontSize, spacing);
    Vector2 szRush  = MeasureTextEx(GetFontDefault(), "RUSH",  (float)titleFontSize, spacing);
    float gap    = 20.0f;
    float totalW = szLogic.x + gap + szRush.x;
    float startX = SCREEN_WIDTH  / 2.0f - totalW / 2.0f;
    float startY = SCREEN_HEIGHT / 4.0f - 30.0f;

    float glowPulse   = 0.5f + 0.5f * sinf(t * 3.0f);
    float glowOpacity = 0.20f + 0.10f * glowPulse;

    for (int dx = -3; dx <= 3; dx += 3) {
        for (int dy = -3; dy <= 3; dy += 3) {
            if (dx == 0 && dy == 0) continue;
            DrawTextEx(GetFontDefault(), "LOGIC",
                       (Vector2){ startX + dx, startY + dy },
                       (float)titleFontSize, spacing,
                       ColorAlpha(COLOR_NEON_PURPLE, glowOpacity * 0.3f));
            DrawTextEx(GetFontDefault(), "RUSH",
                       (Vector2){ startX + szLogic.x + gap + dx, startY + dy },
                       (float)titleFontSize, spacing,
                       ColorAlpha(COLOR_NEON_CYAN, glowOpacity * 0.3f));
        }
    }

    DrawTextEx(GetFontDefault(), "LOGIC", (Vector2){ startX - 1, startY }, (float)titleFontSize, spacing, WHITE);
    DrawTextEx(GetFontDefault(), "LOGIC", (Vector2){ startX + 1, startY }, (float)titleFontSize, spacing, WHITE);
    DrawTextEx(GetFontDefault(), "LOGIC", (Vector2){ startX, startY - 1 }, (float)titleFontSize, spacing, WHITE);
    DrawTextEx(GetFontDefault(), "LOGIC", (Vector2){ startX, startY + 1 }, (float)titleFontSize, spacing, WHITE);
    DrawTextEx(GetFontDefault(), "LOGIC", (Vector2){ startX,     startY }, (float)titleFontSize, spacing, WHITE);

    DrawTextEx(GetFontDefault(), "RUSH", (Vector2){ startX + szLogic.x + gap - 1, startY }, (float)titleFontSize, spacing, COLOR_NEON_CYAN);
    DrawTextEx(GetFontDefault(), "RUSH", (Vector2){ startX + szLogic.x + gap + 1, startY }, (float)titleFontSize, spacing, COLOR_NEON_CYAN);
    DrawTextEx(GetFontDefault(), "RUSH", (Vector2){ startX + szLogic.x + gap, startY - 1 }, (float)titleFontSize, spacing, COLOR_NEON_CYAN);
    DrawTextEx(GetFontDefault(), "RUSH", (Vector2){ startX + szLogic.x + gap, startY + 1 }, (float)titleFontSize, spacing, COLOR_NEON_CYAN);
    DrawTextEx(GetFontDefault(), "RUSH", (Vector2){ startX + szLogic.x + gap, startY     }, (float)titleFontSize, spacing, COLOR_NEON_CYAN);

    const char *sub  = "DESAFIOS DE LÓGICA PROPOSICIONAL";
    int         subW = MeasureText(sub, 20);
    DrawText(sub, SCREEN_WIDTH / 2 - subW / 2,
             SCREEN_HEIGHT / 4 + 82, 20, COLOR_TEXT_MUTED);

    float lineW = 240.0f + sinf(t * 2.0f) * 40.0f;
    DrawLineEx(
        (Vector2){SCREEN_WIDTH / 2.0f - lineW / 2, SCREEN_HEIGHT / 4.0f + 115},
        (Vector2){SCREEN_WIDTH / 2.0f + lineW / 2, SCREEN_HEIGHT / 4.0f + 115},
        1.5f, ColorAlpha(COLOR_NEON_CYAN, 0.35f));

    // 4. Vignette
    DrawThemeVignette(SCREEN_WIDTH, SCREEN_HEIGHT);

    // 5. Botões principais
    DrawThemeButton(storyButton,  "MODO HISTORIA",    18, storyHover,  COLOR_NEON_CYAN);
    DrawThemeButton(competButton, "MODO COMPETITIVO", 18, competHover, COLOR_NEON_GOLD);
    DrawThemeButton(exitButton,   "FECHAR SISTEMA",   18, exitHover,   COLOR_NEON_RED);

    // Subtítulos dos botões
    int sw1 = MeasureText("Narrativa com dialogos — sem pontuacao", 12);
    DrawText("Narrativa com dialogos — sem pontuacao",
             (int)(storyButton.x + storyButton.width / 2 - sw1 / 2),
             (int)(storyButton.y + storyButton.height + 4), 12,
             COLOR_TEXT_MUTED);

    int cw1 = MeasureText("Pontuacao e ranking — sem dialogos", 12);
    DrawText("Pontuacao e ranking — sem dialogos",
             (int)(competButton.x + competButton.width / 2 - cw1 / 2),
             (int)(competButton.y + competButton.height + 4), 12,
             COLOR_TEXT_MUTED);

    // 6. Rodapé
#ifdef __APPLE__
    const char *footer = "Mouse para interagir  |  Ctrl+F: Tela cheia";
#else
    const char *footer = "Mouse para interagir  |  F11: Tela cheia";
#endif
    int         fw     = MeasureText(footer, 14);
    DrawText(footer, SCREEN_WIDTH / 2 - fw / 2,
             SCREEN_HEIGHT - 32, 14, COLOR_TEXT_MUTED);
}

void UnloadTitleScreen(void) {
    // Sem recursos para liberar
}
