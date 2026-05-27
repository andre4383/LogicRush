#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "screens.h"
#include "../dialogue/dialogue.h"

// Screen configuration constants
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

// Active screen state
extern GameScreen currentScreen;

// Modo de jogo atual (Competitivo ou História)
extern GameMode currentGameMode;

// Fonts
extern Font fontMain;
extern Font fontBold;

// Redefine standard Raylib text functions to use Inter font with high quality rendering
static inline void DrawTextCustom(const char* text, int x, int y, int size, Color color) {
    DrawTextEx(fontMain, text, (Vector2){ (float)x, (float)y }, (float)size, 0.0f, color);
}
static inline int MeasureTextCustom(const char* text, int size) {
    return (int)MeasureTextEx(fontMain, text, (float)size, 0.0f).x;
}
static inline void DrawTextBoldCustom(const char* text, int x, int y, int size, Color color) {
    DrawTextEx(fontBold, text, (Vector2){ (float)x, (float)y }, (float)size, 0.0f, color);
}
static inline int MeasureTextBoldCustom(const char* text, int size) {
    return (int)MeasureTextEx(fontBold, text, (float)size, 0.0f).x;
}

#undef DrawText
#undef MeasureText
#define DrawText DrawTextCustom
#define MeasureText MeasureTextCustom
#define DrawTextBold DrawTextBoldCustom
#define MeasureTextBold MeasureTextBoldCustom

// Main game lifecycle functions
void InitGame(void);
void UpdateGame(void);
void DrawGame(void);
void UnloadGame(void);

// Screen-specific lifecycle functions (implemented in menu/menu.c)
void InitTitleScreen(void);
void UpdateTitleScreen(void);
void DrawTitleScreen(void);
void UnloadTitleScreen(void);

// Screen-specific lifecycle functions (implemented in fase_labirinto/fase_labirinto.c)
void InitGameplayScreen(void);
void UpdateGameplayScreen(void);
void DrawGameplayScreen(void);
void UnloadGameplayScreen(void);

// Screen-specific lifecycle functions (implemented in fase_boss/fase_boss.c)
void InitBossScreen(void);
void UpdateBossScreen(void);
void DrawBossScreen(void);
void UnloadBossScreen(void);

// Screen-specific lifecycle functions (implemented in fase_quiz/memory.c or quiz_screen.c)
void InitQuizScreen(void);
void UpdateQuizScreen(void);
void DrawQuizScreen(void);
void UnloadQuizScreen(void);

// Global persistent state across phases
extern float globalTimer;
extern int globalScore;
extern bool gameRunning;
extern bool gamePaused;
extern int  globalLives;  // lives shared across all phases (max 3)

// Phase Banner Transition Overlay System
extern float phaseBannerTimer;
extern const char* phaseBannerTitle;
extern const char* phaseBannerSubtitle;

void StartPhaseBanner(const char* title, const char* subtitle);
void UpdatePhaseBanner(float dt);
void DrawPhaseBanner(void);

// Unified Glassmorphic HUD drawing helpers
void DrawUnifiedHUD(const char* phaseTitle, const char* phaseSubtitle, const char* helpText);
void DrawBottomHUD(const char* commands);

// Ranking screen
void Ranking_ScreenEnter(int score, float time, int phase);
void UpdateRankingScreen(void);
void DrawRankingScreen(void);

// ── Modo História: tela de introdução ────────────────────────────────────────
void InitIntroScreen(void);
void UpdateIntroScreen(void);
void DrawIntroScreen(void);
void UnloadIntroScreen(void);

// ── Flags de estado do Modo História ─────────────────────────────────────────
// (definidas em game.c, usadas pelas fases)
extern bool storyQuizDialogueShown;
extern bool storyMazeDialogueShown;
extern bool storyBossPhase2Shown;
extern bool storyBossPhase3Shown;

// Reseta todas as flags de história (chame ao iniciar nova partida história)
void Story_ResetFlags(void);

#endif // GAME_H
