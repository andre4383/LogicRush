#include "../core/game.h"
#include "../core/screens.h"
#include "../core/theme.h"
#include <stdlib.h>

// ── Botões do menu ──────────────────────────────────────────────────────────────
static Rectangle playButton = {
    SCREEN_WIDTH / 2.0f - 160, SCREEN_HEIGHT / 2.0f + 20, 320, 58
};
static Rectangle exitButton = {
    SCREEN_WIDTH / 2.0f - 160, SCREEN_HEIGHT / 2.0f + 98, 320, 58
};

static bool playHover = false;
static bool exitHover = false;
static float titleGlow = 0.0f;
static int   titleGlowDir = 1;

void InitTitleScreen(void) {
    playHover     = false;
    exitHover     = false;
    titleGlow     = 0.0f;
    titleGlowDir  = 1;
}

void UpdateTitleScreen(void) {
    Vector2 mouse = GetMousePosition();

    playHover = CheckCollisionPointRec(mouse, playButton);
    exitHover = CheckCollisionPointRec(mouse, exitButton);

    // Anima brilho do título
    titleGlow += 0.018f * titleGlowDir;
    if (titleGlow >= 1.0f) { titleGlow = 1.0f; titleGlowDir = -1; }
    else if (titleGlow <= 0.0f) { titleGlow = 0.0f; titleGlowDir = 1; }

    if (playHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        globalTimer = 0.0f;
        globalScore = 0;
        gameRunning = true;
        gamePaused = false;
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

    // 3. Título com glow cibernético encorpado (usando a fonte padrão "virtual")
    int titleFontSize = 90;
    float spacing = (float)titleFontSize / 10.0f;
    Vector2 szLogic = MeasureTextEx(GetFontDefault(), "LOGIC", (float)titleFontSize, spacing);
    Vector2 szRush = MeasureTextEx(GetFontDefault(), "RUSH", (float)titleFontSize, spacing);
    float gap = 20.0f;
    float totalW = szLogic.x + gap + szRush.x;
    float startX = SCREEN_WIDTH / 2.0f - totalW / 2.0f;
    float startY = SCREEN_HEIGHT / 4.0f - 30.0f;

    // Efeito de neon bloom por camadas
    float glowPulse = 0.5f + 0.5f * sinf(t * 3.0f);
    float glowOpacity = 0.20f + 0.10f * glowPulse;

    // Camada de brilho externa
    for (int dx = -3; dx <= 3; dx += 3) {
        for (int dy = -3; dy <= 3; dy += 3) {
            if (dx == 0 && dy == 0) continue;
            DrawTextEx(GetFontDefault(), "LOGIC", (Vector2){ startX + dx, startY + dy }, (float)titleFontSize, spacing, ColorAlpha(COLOR_NEON_PURPLE, glowOpacity * 0.3f));
            DrawTextEx(GetFontDefault(), "RUSH", (Vector2){ startX + szLogic.x + gap + dx, startY + dy }, (float)titleFontSize, spacing, ColorAlpha(COLOR_NEON_CYAN, glowOpacity * 0.3f));
        }
    }

    // Desenha o texto principal em negrito encorpado (Thick Virtual Font)
    // Para LOGIC (Branco)
    DrawTextEx(GetFontDefault(), "LOGIC", (Vector2){ startX - 1, startY }, (float)titleFontSize, spacing, WHITE);
    DrawTextEx(GetFontDefault(), "LOGIC", (Vector2){ startX + 1, startY }, (float)titleFontSize, spacing, WHITE);
    DrawTextEx(GetFontDefault(), "LOGIC", (Vector2){ startX, startY - 1 }, (float)titleFontSize, spacing, WHITE);
    DrawTextEx(GetFontDefault(), "LOGIC", (Vector2){ startX, startY + 1 }, (float)titleFontSize, spacing, WHITE);
    DrawTextEx(GetFontDefault(), "LOGIC", (Vector2){ startX, startY }, (float)titleFontSize, spacing, WHITE);

    // Para RUSH (Ciano Neon)
    DrawTextEx(GetFontDefault(), "RUSH", (Vector2){ startX + szLogic.x + gap - 1, startY }, (float)titleFontSize, spacing, COLOR_NEON_CYAN);
    DrawTextEx(GetFontDefault(), "RUSH", (Vector2){ startX + szLogic.x + gap + 1, startY }, (float)titleFontSize, spacing, COLOR_NEON_CYAN);
    DrawTextEx(GetFontDefault(), "RUSH", (Vector2){ startX + szLogic.x + gap, startY - 1 }, (float)titleFontSize, spacing, COLOR_NEON_CYAN);
    DrawTextEx(GetFontDefault(), "RUSH", (Vector2){ startX + szLogic.x + gap, startY + 1 }, (float)titleFontSize, spacing, COLOR_NEON_CYAN);
    DrawTextEx(GetFontDefault(), "RUSH", (Vector2){ startX + szLogic.x + gap, startY }, (float)titleFontSize, spacing, COLOR_NEON_CYAN);

    // Subtítulo
    const char *sub  = "DESAFIOS DE LÓGICA PROPOSICIONAL";
    int         subW = MeasureText(sub, 20);
    DrawText(sub, SCREEN_WIDTH / 2 - subW / 2,
             SCREEN_HEIGHT / 4 + 82, 20, COLOR_TEXT_MUTED);

    // Linha separadora animada
    float lineW = 240.0f + sinf(t * 2.0f) * 40.0f;
    DrawLineEx(
        (Vector2){SCREEN_WIDTH / 2.0f - lineW / 2, SCREEN_HEIGHT / 4.0f + 115},
        (Vector2){SCREEN_WIDTH / 2.0f + lineW / 2, SCREEN_HEIGHT / 4.0f + 115},
        1.5f, ColorAlpha(COLOR_NEON_CYAN, 0.35f));

    // 4. Vignette
    DrawThemeVignette(SCREEN_WIDTH, SCREEN_HEIGHT);

    // 5. Botões
    DrawThemeButton(playButton, "JOGAR",   18, playHover, COLOR_NEON_CYAN);
    DrawThemeButton(exitButton, "FECHAR SISTEMA",       18, exitHover, COLOR_NEON_RED);

    // Labels dos botões
    int lw1 = MeasureText("Iniciar Desafios de Lógica Proposicional", 13);
    DrawText("Iniciar Desafios de Lógica Proposicional",
             (int)(playButton.x + playButton.width / 2 - lw1 / 2),
             (int)(playButton.y + playButton.height + 4), 13, COLOR_TEXT_MUTED);

    // Rodapé
    const char *footer = "Utilize o mouse para interagir";
    int         fw     = MeasureText(footer, 14);
    DrawText(footer, SCREEN_WIDTH / 2 - fw / 2,
             SCREEN_HEIGHT - 32, 14, COLOR_TEXT_MUTED);
}

void UnloadTitleScreen(void) {
    // Sem recursos para liberar
}
