#include "../core/game.h"
#include "../core/ranking.h"
#include "../core/screens.h"
#include "../core/theme.h"
#include <stdlib.h>

// ── Botões do menu principal ─────────────────────────────────────────────────
static Rectangle playButton = {
    SCREEN_WIDTH / 2.0f - 160, SCREEN_HEIGHT / 2.0f + 20, 320, 58
};
static Rectangle exitButton = {
    SCREEN_WIDTH / 2.0f - 160, SCREEN_HEIGHT / 2.0f + 98, 320, 58
};

// ── Botões de pulo rápido de fase (modo teste) ───────────────────────────────
//    Ficam no canto inferior esquerdo, pequenos e discretos
#define DBG_BTN_W  110
#define DBG_BTN_H   34
#define DBG_BTN_X   18
#define DBG_BTN_GAP  8

static Rectangle dbgFase1Btn = { DBG_BTN_X,
    SCREEN_HEIGHT - DBG_BTN_H - DBG_BTN_GAP*2 - DBG_BTN_H*2,
    DBG_BTN_W, DBG_BTN_H };

static Rectangle dbgFase2Btn = { DBG_BTN_X,
    SCREEN_HEIGHT - DBG_BTN_H - DBG_BTN_GAP - DBG_BTN_H,
    DBG_BTN_W, DBG_BTN_H };

static Rectangle dbgFase3Btn = { DBG_BTN_X,
    SCREEN_HEIGHT - DBG_BTN_H,
    DBG_BTN_W, DBG_BTN_H };

static bool playHover  = false;
static bool exitHover  = false;
static bool dbg1Hover  = false;
static bool dbg2Hover  = false;
static bool dbg3Hover  = false;
static float titleGlow = 0.0f;
static int   titleGlowDir = 1;

void InitTitleScreen(void) {
    playHover    = false;
    exitHover    = false;
    dbg1Hover    = false;
    dbg2Hover    = false;
    dbg3Hover    = false;
    titleGlow    = 0.0f;
    titleGlowDir = 1;
}

void UpdateTitleScreen(void) {
    Vector2 mouse = GetMousePosition();

    playHover = CheckCollisionPointRec(mouse, playButton);
    exitHover = CheckCollisionPointRec(mouse, exitButton);
    dbg1Hover = CheckCollisionPointRec(mouse, dbgFase1Btn);
    dbg2Hover = CheckCollisionPointRec(mouse, dbgFase2Btn);
    dbg3Hover = CheckCollisionPointRec(mouse, dbgFase3Btn);

    // Anima brilho do título
    titleGlow += 0.018f * titleGlowDir;
    if      (titleGlow >= 1.0f) { titleGlow = 1.0f; titleGlowDir = -1; }
    else if (titleGlow <= 0.0f) { titleGlow = 0.0f; titleGlowDir =  1; }

    // ── Botão JOGAR (fluxo normal: Fase 1) ──────────────────────────────────
    if (playHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        globalTimer = 0.0f;
        globalScore = 0;
        globalLives = 3;  // reset lives for new game
        gameRunning = true;
        gamePaused  = false;
        InitQuizScreen();
        currentScreen = SCREEN_QUIZ;
        StartPhaseBanner("FASE 1", "QUIZ DE EQUIVALÊNCIAS");
    }

    // ── Botão debug FASE 1 ───────────────────────────────────────────────────
    if (dbg1Hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        globalTimer = 0.0f;
        globalScore = 0;
        globalLives = 3;  // reset lives for new game
        gameRunning = true;
        gamePaused  = false;
        InitQuizScreen();
        currentScreen = SCREEN_QUIZ;
        StartPhaseBanner("FASE 1", "QUIZ DE EQUIVALÊNCIAS");
    }

    // ── Botão debug FASE 2 ───────────────────────────────────────────────────
    if (dbg2Hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        globalTimer = 0.0f;
        globalScore = 0;
        gameRunning = true;
        gamePaused  = false;
        InitGameplayScreen();
        currentScreen = SCREEN_GAMEPLAY;
        StartPhaseBanner("FASE 2", "LABIRINTO DE CABOS");
    }

    // ── Botão debug FASE 3 ───────────────────────────────────────────────────
    if (dbg3Hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        globalTimer = 0.0f;
        globalScore = 0;
        gameRunning = true;
        gamePaused  = false;
        InitBossScreen();
        currentScreen = SCREEN_BOSS;
        StartPhaseBanner("FASE 3", "MEETING EQUAL");
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
    DrawThemeButton(playButton, "JOGAR",          18, playHover, COLOR_NEON_CYAN);
    DrawThemeButton(exitButton, "FECHAR SISTEMA", 18, exitHover, COLOR_NEON_RED);

    int lw1 = MeasureText("Iniciar Desafios de Lógica Proposicional", 13);
    DrawText("Iniciar Desafios de Lógica Proposicional",
             (int)(playButton.x + playButton.width / 2 - lw1 / 2),
             (int)(playButton.y + playButton.height + 4), 13, COLOR_TEXT_MUTED);

    // 6. Painel de debug (canto inferior esquerdo) ────────────────────────────
    // Fundo do painel
    Rectangle dbgPanel = {
        DBG_BTN_X - 6,
        dbgFase1Btn.y - 28,
        DBG_BTN_W + 12,
        DBG_BTN_H * 3 + DBG_BTN_GAP * 2 + 28
    };
    DrawRectangleRec(dbgPanel, ColorAlpha(COLOR_BG_DARK, 0.80f));
    DrawRectangleLinesEx(dbgPanel, 1.0f, ColorAlpha(COLOR_NEON_GOLD, 0.35f));

    // Label "[TESTE]"
    DrawText("[TESTE]",
             (int)(dbgPanel.x + 6),
             (int)(dbgFase1Btn.y - 20),
             11, ColorAlpha(COLOR_NEON_GOLD, 0.70f));

    // Botões de pulo rápido
    // FASE 1 — ciano
    DrawRectangleRec(dbgFase1Btn,
        dbg1Hover ? ColorAlpha(COLOR_NEON_CYAN, 0.28f) : ColorAlpha(COLOR_BG_DARK, 0.90f));
    DrawRectangleLinesEx(dbgFase1Btn, 1.0f,
        dbg1Hover ? COLOR_NEON_CYAN : ColorAlpha(COLOR_NEON_CYAN, 0.45f));
    DrawText("FASE 1",
             (int)(dbgFase1Btn.x + DBG_BTN_W / 2 - MeasureText("FASE 1", 13) / 2),
             (int)(dbgFase1Btn.y + DBG_BTN_H / 2 - 7),
             13, dbg1Hover ? WHITE : ColorAlpha(COLOR_NEON_CYAN, 0.85f));

    // FASE 2 — verde
    DrawRectangleRec(dbgFase2Btn,
        dbg2Hover ? ColorAlpha(COLOR_NEON_GREEN, 0.28f) : ColorAlpha(COLOR_BG_DARK, 0.90f));
    DrawRectangleLinesEx(dbgFase2Btn, 1.0f,
        dbg2Hover ? COLOR_NEON_GREEN : ColorAlpha(COLOR_NEON_GREEN, 0.45f));
    DrawText("FASE 2",
             (int)(dbgFase2Btn.x + DBG_BTN_W / 2 - MeasureText("FASE 2", 13) / 2),
             (int)(dbgFase2Btn.y + DBG_BTN_H / 2 - 7),
             13, dbg2Hover ? WHITE : ColorAlpha(COLOR_NEON_GREEN, 0.85f));

    // FASE 3 — vermelho/pink (boss)
    DrawRectangleRec(dbgFase3Btn,
        dbg3Hover ? ColorAlpha(COLOR_NEON_RED, 0.28f) : ColorAlpha(COLOR_BG_DARK, 0.90f));
    DrawRectangleLinesEx(dbgFase3Btn, 1.0f,
        dbg3Hover ? COLOR_NEON_RED : ColorAlpha(COLOR_NEON_RED, 0.45f));
    DrawText("FASE 3",
             (int)(dbgFase3Btn.x + DBG_BTN_W / 2 - MeasureText("FASE 3", 13) / 2),
             (int)(dbgFase3Btn.y + DBG_BTN_H / 2 - 7),
             13, dbg3Hover ? WHITE : ColorAlpha(COLOR_NEON_RED, 0.85f));

    // 7. Rodapé
    const char *footer = "Mouse para interagir  |  F11: Tela cheia";
    int         fw     = MeasureText(footer, 14);
    DrawText(footer, SCREEN_WIDTH / 2 - fw / 2,
             SCREEN_HEIGHT - 32, 14, COLOR_TEXT_MUTED);
}

void UnloadTitleScreen(void) {
    // Sem recursos para liberar
}
