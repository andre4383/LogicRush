#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "screens.h"

// Screen configuration constants
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

// Active screen state
extern GameScreen currentScreen;

// Fonts
extern Font fontMain;
extern Font fontBold;

// Redefine standard Raylib text functions to use Inter font with high quality rendering
static inline void DrawTextCustom(const char* text, int x, int y, int size, Color color) {
    DrawTextEx(fontMain, text, (Vector2){ (float)x, (float)y }, (float)size, 1.0f, color);
}
static inline int MeasureTextCustom(const char* text, int size) {
    return (int)MeasureTextEx(fontMain, text, (float)size, 1.0f).x;
}
static inline void DrawTextBoldCustom(const char* text, int x, int y, int size, Color color) {
    DrawTextEx(fontBold, text, (Vector2){ (float)x, (float)y }, (float)size, 1.0f, color);
}
static inline int MeasureTextBoldCustom(const char* text, int size) {
    return (int)MeasureTextEx(fontBold, text, (float)size, 1.0f).x;
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

#endif // GAME_H
