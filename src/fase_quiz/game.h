#ifndef QUIZ_GAME_H
#define QUIZ_GAME_H

#include "../core/game.h"
#include <stdbool.h>

// ─── Screen ────────────────────────────────────────────────────────────────
#define SCREEN_W   1280
#define SCREEN_H   720
#define TARGET_FPS  60
#define GAME_TITLE "Logic Rush"

// ─── Colours ───────────────────────────────────────────────────────────────
#define COL_BG        CLITERAL(Color){10,  12,  28,  255}
#define COL_PANEL     CLITERAL(Color){18,  22,  48,  255}
#define COL_ACCENT    CLITERAL(Color){0,   220, 220, 255}   // Equal cyan
#define COL_EVIL      CLITERAL(Color){220, 30,  30,  255}   // Equal evil red
#define COL_CARD_BACK CLITERAL(Color){30,  40,  80,  255}
#define COL_CARD_FACE CLITERAL(Color){220, 235, 255, 255}
#define COL_CORRECT   CLITERAL(Color){50,  220, 100, 255}
#define COL_WRONG     CLITERAL(Color){220, 60,  60,  255}
#define COL_TEXT      CLITERAL(Color){230, 230, 255, 255}
#define COL_DIM       CLITERAL(Color){100, 110, 160, 255}

// ─── Game State ────────────────────────────────────────────────────────────
typedef enum {
    STATE_TITLE = 0,
    STATE_MINIGAME_MEMORY,
    STATE_MINIGAME_SORT,     // placeholder
    STATE_MINIGAME_CIRCUIT,  // placeholder
    STATE_TRANSITION,
    STATE_GAME_OVER,
    STATE_VICTORY
} GameState;

// ─── Equal moods ───────────────────────────────────────────────────────────
typedef enum {
    EQUAL_NORMAL = 0,
    EQUAL_ANGRY,
    EQUAL_GLITCH
} EqualMood;

// ─── Shared context passed everywhere ──────────────────────────────────────
typedef struct {
    GameState   state;
    GameState   nextState;
    int         cycle;          // how many full minigame cycles done
    int         lives;
    float       transTimer;     // seconds remaining in transition
    EqualMood   mood;
    float       moodTimer;      // glitch shake duration
    float       shakeX, shakeY; // current shake offset
    bool        paused;
} GameCtx;

// ─── Forward declarations ──────────────────────────────────────────────────
void TitleScreen_Update(GameCtx *ctx);
void TitleScreen_Draw(GameCtx *ctx);

void MemoryGame_Init(GameCtx *ctx);
void MemoryGame_Update(GameCtx *ctx, float dt);
void MemoryGame_Draw(GameCtx *ctx);
void MemoryGame_Free(void);

void Equal_Draw(GameCtx *ctx, float x, float y, float scale);
void Equal_UpdateMood(GameCtx *ctx, float dt);

void Transition_Update(GameCtx *ctx, float dt);
void Transition_Draw(GameCtx *ctx);

void GameOver_Draw(GameCtx *ctx);
void Victory_Draw(GameCtx *ctx);

// ─── Utility ───────────────────────────────────────────────────────────────
float RandFloat(float lo, float hi);   // random float in [lo, hi]
void  DrawTextCentered(Font font, const char *text, float y,
                       float fontSize, float spacing, Color tint);

#endif // QUIZ_GAME_H