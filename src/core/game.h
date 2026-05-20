#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "screens.h"

// Screen configuration constants
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

// Active screen state
extern GameScreen currentScreen;

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
