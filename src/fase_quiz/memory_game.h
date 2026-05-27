#ifndef MEMORY_GAME_H
#define MEMORY_GAME_H

#include "game.h"

// ─── Symbols ───────────────────────────────────────────────────────────────
// Each card has one of these logic-equivalence symbols
typedef enum {
    SYM_EQUIV = 0,  // ≡  (triple bar)
    SYM_IMPL,       // →
    SYM_BICONDIT,   // ↔
    SYM_XOR,        // ⊕
    SYM_NAND,       // ↑
    SYM_NOR,        // ↓
    SYM_COUNT
} Symbol;

// ─── Card ──────────────────────────────────────────────────────────────────
typedef enum {
    CARD_FACE_DOWN = 0,
    CARD_FLIPPING_UP,
    CARD_FACE_UP,
    CARD_FLIPPING_DOWN,
    CARD_MATCHED
} CardState;

typedef struct {
    Symbol     symbol;
    CardState  state;
    float      flipT;    // 0..1 animation progress
    float      x, y;     // centre position
    float      width, height;
    bool       selected;
    float      matchSmokeTimer;
} Card;

// ─── Memory game state ─────────────────────────────────────────────────────
#define MG_MAX_CARDS 24

typedef enum {
    MG_PHASE_SHOW = 0,   // briefly show all cards
    MG_PHASE_HIDE,       // flip them all back
    MG_PHASE_PLAY,       // player choosing
    MG_PHASE_CHECK,      // brief pause after two selected
    MG_PHASE_RESULT,     // show pass/fail
    MG_PHASE_DONE        // handoff to GameCtx
} MgPhase;

typedef struct {
    Card    cards[MG_MAX_CARDS];
    int     cardCount;
    int     pairsTotal;
    int     pairsFound;
    int     pairsNeeded;   // how many pairs to pass (varies per cycle)
    int     sel[2];        // indices of the two selected cards (-1 = none)
    int     selCount;
    MgPhase phase;
    float   phaseTimer;
    bool    passed;
    int     wrongStreak;      // equal gets angrier with wrong streaks
    float   wrongFlashT;      // white flash on wrong match
    float   correctFlashT;
    float   playTimeLeft;
    float   playTimeMax;
    int     correctStreak;    // consecutive correct pairs for life-gain combo
    float   comboTimer;       // time window between pairs; resets streak when 0
    float   comboFeedbackTimer; // shows "+1 VIDA!" feedback when > 0
} MemoryGameState;

extern MemoryGameState mgState;

#endif // MEMORY_GAME_H