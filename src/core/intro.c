/*
 * intro.c — Tela de Introdução do Modo História — Logic Rush
 *
 * Sequência:
 *   1. Tela preta: digita "Link estabelecido. Cyber-Astro online" em azul ciano
 *   2. Pausa 1s
 *   3. Diálogos de introdução (Astro / Equal / Astro)
 *   4. Tela preta: digita "Fase 1: A Matriz de Memoria (Jogo da Memoria)"
 *   5. Pausa 1s
 *   6. Popup de contexto da Fase 1 (glassmorphic)
 *   7. Diálogos pré-fase 1 (Astro / Equal)
 *   8. Inicia SCREEN_QUIZ
 */

#include "game.h"
#include "screens.h"
#include "theme.h"
#include "../dialogue/dialogue.h"
#include "raylib.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

// ── Sub-estados ───────────────────────────────────────────────────────────────
typedef enum {
    IS_TYPING_CONNECT = 0,  // digitação "Link estabelecido..."
    IS_CONNECT_PAUSE,       // pausa 1s após digitação
    IS_DIALOGUE_INTRO,      // 3 diálogos de abertura
    IS_TYPING_PHASE,        // digitação "Fase 1: ..."
    IS_PHASE_PAUSE,         // pausa 1s
    IS_POPUP_FASE1,         // popup de contexto Fase 1
    IS_DIALOGUE_PRE1,       // diálogos pré-fase 1
    IS_DONE                 // inicia quiz
} IntroSubState;

static IntroSubState introState = IS_TYPING_CONNECT;
static TypingState   typing     = {0};
static float         pauseTimer = 0.0f;
static bool          popupBtnHover = false;

static const char *TEXT_CONNECT = "Link estabelecido. Cyber-Astro online";
static const char *TEXT_PHASE   = "Fase 1: A Matriz de Memoria (Jogo da Memoria)";

// Posição do botão do popup (usada em Update e Draw)
static const Rectangle POPUP_BTN = { SCREEN_WIDTH/2.0f - 100,
                                     SCREEN_HEIGHT/2.0f + 140, 200, 42 };

// ── Utilidade: desenha fundo preto ────────────────────────────────────────────
static void DrawBlackBg(void) {
    ClearBackground(BLACK);
}

// ── Popup de contexto Fase 1 ─────────────────────────────────────────────────
static void DrawFase1Popup(void) {
    // Overlay escurecido
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha(BLACK, 0.78f));

    float popW = 650, popH = 380;
    float popX = SCREEN_WIDTH  / 2.0f - popW / 2.0f;
    float popY = SCREEN_HEIGHT / 2.0f - popH / 2.0f;
    Rectangle panel = { popX, popY, popW, popH };

    DrawThemeGlassPanel(panel, 0.06f, COLOR_NEON_CYAN);

    // Título
    const char *title = "A Matriz de Memoria (Jogo da Memoria)";
    int tw = MeasureTextBold(title, 20);
    DrawTextBold(title, (int)(popX + popW/2 - tw/2), (int)(popY + 18), 20, COLOR_NEON_CYAN);

    // Separador
    DrawLineEx((Vector2){popX + 30, popY + 50},
               (Vector2){popX + popW - 30, popY + 50},
               1.5f, ColorAlpha(COLOR_NEON_CYAN, 0.4f));

    // Inimigo da Fase
    const char *enemy = "Inimigo: Nodes de Cache Corrompidos";
    int ew = MeasureText(enemy, 14);
    DrawText(enemy, (int)(popX + popW/2 - ew/2), (int)(popY + 60), 14,
             ColorAlpha(COLOR_NEON_PURPLE, 0.9f));

    // Descrição
    float cx = popX + 26, cy = popY + 90;
    int   fs = 13;
    float lh = 19.0f;
    DrawText("A Equal protege seus protocolos com criptografia de portas logicas.", (int)cx, (int)cy,        fs, COLOR_TEXT_MUTED);
    DrawText("Encontre as equivalencias exatas para criar um curto-circuito e queimar", (int)cx, (int)(cy+lh),  fs, COLOR_TEXT_MUTED);
    DrawText("o hardware de defesa dela (Stack Overflow).",                              (int)cx, (int)(cy+lh*2),fs, COLOR_TEXT_MUTED);

    DrawLineEx((Vector2){popX + 30, popY + 165},
               (Vector2){popX + popW - 30, popY + 165},
               1.0f, ColorAlpha(COLOR_PANEL_BORDER, 0.5f));

    DrawTextBold("OBJETIVO:", (int)(cx), (int)(popY + 175), 13, COLOR_TEXT_CYBER);
    DrawText("Combine todos os pares de cartas com proposicoes equivalentes.",     (int)cx, (int)(popY+195), 13, COLOR_TEXT_MUTED);
    DrawText("Cada erro custa uma vida. Encontre todos os pares para vencer!",    (int)cx, (int)(popY+213), 13, COLOR_TEXT_MUTED);

    // Botão
    Vector2 mp = GetMousePosition();
    popupBtnHover = CheckCollisionPointRec(mp, POPUP_BTN);
    DrawThemeButton(POPUP_BTN, "Entendido", 16, popupBtnHover, COLOR_NEON_CYAN);
}

// ─────────────────────────────────────────────────────────────────────────────

void InitIntroScreen(void) {
    introState    = IS_TYPING_CONNECT;
    pauseTimer    = 0.0f;
    popupBtnHover = false;
    Typing_Start(&typing, TEXT_CONNECT, 0.045f);
}

void UnloadIntroScreen(void) {
    // Sem recursos extras a liberar
}

void UpdateIntroScreen(void) {
    float dt = GetFrameTime();

    switch (introState) {

    // ── 1. Digitação do texto de conexão ─────────────────────────────────────
    case IS_TYPING_CONNECT:
        Typing_Update(&typing, dt);
        // Enter/Espaço pula ou avança
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
            Typing_Skip(&typing);
        // Quando digitação terminar → pausa
        if (Typing_IsDone(&typing)) {
            introState = IS_CONNECT_PAUSE;
            pauseTimer = 1.2f;
        }
        break;

    // ── 2. Pausa após conexão ─────────────────────────────────────────────────
    case IS_CONNECT_PAUSE:
        pauseTimer -= dt;
        if (pauseTimer <= 0.0f || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            introState = IS_DIALOGUE_INTRO;
            Dialogue_StartSeq(DSEQ_INTRO, SCREEN_INTRO); // quando terminar, volta a SCREEN_INTRO
        }
        break;

    // ── 3. Diálogos de introdução ─────────────────────────────────────────────
    case IS_DIALOGUE_INTRO:
        Dialogue_Update();
        if (!Dialogue_IsActive()) {
            // currentScreen foi setado para SCREEN_INTRO pelo Dialogue → sem problema
            // Avançamos internamente para próxima sub-etapa
            introState = IS_TYPING_PHASE;
            Typing_Start(&typing, TEXT_PHASE, 0.05f);
        }
        break;

    // ── 4. Digitação do título da Fase 1 ──────────────────────────────────────
    case IS_TYPING_PHASE:
        Typing_Update(&typing, dt);
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
            Typing_Skip(&typing);
        if (Typing_IsDone(&typing)) {
            introState = IS_PHASE_PAUSE;
            pauseTimer = 1.0f;
        }
        break;

    // ── 5. Pausa após título ──────────────────────────────────────────────────
    case IS_PHASE_PAUSE:
        pauseTimer -= dt;
        if (pauseTimer <= 0.0f || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
            introState = IS_POPUP_FASE1;
        break;

    // ── 6. Popup de contexto Fase 1 ───────────────────────────────────────────
    case IS_POPUP_FASE1: {
        Vector2 mp = GetMousePosition();
        popupBtnHover = CheckCollisionPointRec(mp, POPUP_BTN);
        if ((popupBtnHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            introState = IS_DIALOGUE_PRE1;
            Dialogue_StartSeq(DSEQ_PRE_FASE1, SCREEN_INTRO);
        }
        break;
    }

    // ── 7. Diálogos pré-fase 1 ────────────────────────────────────────────────
    case IS_DIALOGUE_PRE1:
        Dialogue_Update();
        if (!Dialogue_IsActive()) {
            storyQuizDialogueShown = true;
            introState = IS_DONE;
        }
        break;

    // ── 8. Lança Fase 1 ───────────────────────────────────────────────────────
    case IS_DONE:
        InitQuizScreen();
        currentScreen = SCREEN_QUIZ;
        StartPhaseBanner("FASE 1", "A MATRIZ DE MEMORIA");
        break;
    }
}

void DrawIntroScreen(void) {
    switch (introState) {

    // Digitação "Link estabelecido..." e pausa
    case IS_TYPING_CONNECT:
    case IS_CONNECT_PAUSE: {
        DrawBlackBg();
        const char *txt = Typing_Text(&typing);
        int tw = MeasureText(txt, 28);
        int x  = SCREEN_WIDTH  / 2 - tw / 2;
        int y  = SCREEN_HEIGHT / 2 - 14;
        // Glow
        DrawText(txt, x + 2, y + 2, 28, ColorAlpha(COLOR_NEON_CYAN, 0.25f));
        DrawText(txt, x, y, 28, COLOR_NEON_CYAN);
        // Cursor piscante
        if (!Typing_IsDone(&typing) && (int)(GetTime()*2) % 2 == 0) {
            char cur[2] = {95, 0}; // '_'
            DrawText(cur, x + tw + 3, y, 28, COLOR_NEON_CYAN);
        }
        break;
    }

    // Diálogos de introdução (sobre fundo preto)
    case IS_DIALOGUE_INTRO: {
        DrawBlackBg();
        // Texto de conexão permanece ao fundo, mais apagado
        int tw = MeasureText(TEXT_CONNECT, 20);
        DrawText(TEXT_CONNECT, SCREEN_WIDTH/2 - tw/2, 80, 20,
                 ColorAlpha(COLOR_NEON_CYAN, 0.25f));
        Dialogue_Draw();
        break;
    }

    // Digitação título Fase 1 e pausa
    case IS_TYPING_PHASE:
    case IS_PHASE_PAUSE: {
        DrawBlackBg();
        const char *txt = Typing_Text(&typing);
        int tw = MeasureText(txt, 26);
        int x  = SCREEN_WIDTH  / 2 - tw / 2;
        int y  = SCREEN_HEIGHT / 2 - 13;
        DrawText(txt, x + 2, y + 2, 26, ColorAlpha(COLOR_TEXT_MAIN, 0.20f));
        DrawText(txt, x, y, 26, COLOR_TEXT_MAIN);
        if (!Typing_IsDone(&typing) && (int)(GetTime()*2) % 2 == 0) {
            char cur[2] = {95, 0};
            DrawText(cur, x + tw + 3, y, 26, COLOR_TEXT_MAIN);
        }
        break;
    }

    // Popup de contexto Fase 1
    case IS_POPUP_FASE1: {
        DrawBlackBg();
        // Título da fase ao fundo
        int tw = MeasureText(TEXT_PHASE, 16);
        DrawText(TEXT_PHASE, SCREEN_WIDTH/2 - tw/2, 30, 16,
                 ColorAlpha(COLOR_TEXT_MUTED, 0.45f));
        DrawFase1Popup();
        break;
    }

    // Diálogos pré-fase 1 (sobre fundo com título)
    case IS_DIALOGUE_PRE1: {
        DrawBlackBg();
        int tw = MeasureText(TEXT_PHASE, 16);
        DrawText(TEXT_PHASE, SCREEN_WIDTH/2 - tw/2, 30, 16,
                 ColorAlpha(COLOR_TEXT_MUTED, 0.35f));
        Dialogue_Draw();
        break;
    }

    case IS_DONE:
        DrawBlackBg();
        break;
    }
}
