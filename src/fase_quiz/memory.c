/*
 * Logic Rush — memory_game.c
 *
 * Minigame 1: Cartas de Memória
 *  - Cards laid in a grid, briefly shown face-up, then flipped.
 *  - Player clicks two cards to try to match symbols.
 *  - Must find `pairsNeeded` pairs to pass (increases each cycle).
 *  - Equal reacts to wrong matches.
 */

#include "raylib.h"
#include "game.h"
#include "memory_game.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

// ── Global state ─────────────────────────────────────────────────────────
MemoryGameState mgState = {0};

// ── Symbol text labels ───────────────────────────────────────────────────
static const char *SYMBOL_LABELS[SYM_COUNT] = {
    " = ",   // SYM_EQUIV     (≡)
    "->",    // SYM_IMPL      (→)
    "<->",   // SYM_BICONDIT  (↔)
    "XOR",   // SYM_XOR       (⊕)
    "NAND",  // SYM_NAND      (↑)
    " NOR",  // SYM_NOR       (↓)
};

// Symbol colours for extra flair
static const Color SYMBOL_COLORS[SYM_COUNT] = {
    {0,   220, 220, 255},
    {100, 200, 255, 255},
    {200, 100, 255, 255},
    {255, 200,  50, 255},
    {255, 100, 100, 255},
    {100, 255, 150, 255},
};

// ── Fisher-Yates shuffle ─────────────────────────────────────────────────
static void ShuffleInts(int *arr, int n)
{
    for (int i = n - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp;
    }
}

// ── Layout helpers ───────────────────────────────────────────────────────
static void LayoutCards(int count)
{
    // Choose cols/rows based on count
    int cols, rows;
    if (count <= 8)       { cols = 4; rows = 2; }
    else if (count <= 12) { cols = 4; rows = 3; }
    else if (count <= 16) { cols = 4; rows = 4; }
    else                  { cols = 6; rows = 4; }

    float cardW = 90.0f;
    float cardH = 70.0f;
    float padX  = 12.0f;
    float padY  = 12.0f;

    float totalW = cols * (cardW + padX) - padX;
    float totalH = rows * (cardH + padY) - padY;

    // Centre the grid, leaving right side for Equal
    float startX = (SCREEN_W * 0.60f - totalW) * 0.5f + 20;
    float startY = (SCREEN_H - totalH) * 0.5f;

    for (int i = 0; i < count; i++)
    {
        int c = i % cols;
        int r = i / cols;
        mgState.cards[i].x      = startX + c * (cardW + padX) + cardW * 0.5f;
        mgState.cards[i].y      = startY + r * (cardH + padY) + cardH * 0.5f;
        mgState.cards[i].width  = cardW;
        mgState.cards[i].height = cardH;
    }
}

// ── Init ─────────────────────────────────────────────────────────────────
void MemoryGame_Init(GameCtx *ctx)
{
    srand((unsigned int)time(NULL));
    memset(&mgState, 0, sizeof(mgState));

    // Scale difficulty with cycle:
    // cycle 0 → 4 pairs (8 cards), cycle 1 → 6 pairs, cycle 2+ → 8 pairs
    int pairs;
    if (ctx->cycle == 0)      pairs = 4;
    else if (ctx->cycle == 1) pairs = 6;
    else                      pairs = 8;

    // pairsNeeded: must find at least ceil(pairs * 0.6) to pass
    mgState.pairsTotal  = pairs;
    mgState.cardCount   = pairs * 2;
    mgState.pairsNeeded = (pairs * 3 + 4) / 5; // ~60 %
    mgState.pairsFound  = 0;
    mgState.sel[0]      = -1;
    mgState.sel[1]      = -1;
    mgState.selCount    = 0;
    mgState.phase       = MG_PHASE_SHOW;
    mgState.passed      = false;
    mgState.wrongStreak = 0;

    // Show time shortens with cycles
    float showTime = 2.5f - ctx->cycle * 0.3f;
    if (showTime < 0.8f) showTime = 0.8f;
    mgState.phaseTimer  = showTime;

    // Build symbol list (pairs of symbols)
    int symbolAssign[MG_MAX_CARDS];
    // Use random subset of symbols
    int symPool[SYM_COUNT] = {0,1,2,3,4,5};
    ShuffleInts(symPool, SYM_COUNT);

    for (int i = 0; i < pairs; i++)
    {
        symbolAssign[i*2]   = symPool[i % SYM_COUNT];
        symbolAssign[i*2+1] = symPool[i % SYM_COUNT];
    }
    ShuffleInts(symbolAssign, pairs * 2);

    for (int i = 0; i < mgState.cardCount; i++)
    {
        mgState.cards[i].symbol   = (Symbol)symbolAssign[i];
        mgState.cards[i].state    = CARD_FACE_UP;   // will flip down
        mgState.cards[i].flipT    = 1.0f;
        mgState.cards[i].selected = false;
    }

    LayoutCards(mgState.cardCount);
}

// ── Free ─────────────────────────────────────────────────────────────────
void MemoryGame_Free(void)
{
    // Nothing heap-allocated currently; reserved for future audio/texture
}

// ── Flip animation helpers ────────────────────────────────────────────────
#define FLIP_SPEED 6.0f

static void StartFlipUp(int idx)
{
    mgState.cards[idx].state = CARD_FLIPPING_UP;
    mgState.cards[idx].flipT = 0.0f;
}

static void StartFlipDown(int idx)
{
    mgState.cards[idx].state = CARD_FLIPPING_DOWN;
    mgState.cards[idx].flipT = 1.0f;
}

// ── Update ───────────────────────────────────────────────────────────────
void MemoryGame_Update(GameCtx *ctx, float dt)
{
    // Animate card flips
    for (int i = 0; i < mgState.cardCount; i++)
    {
        Card *c = &mgState.cards[i];
        if (c->state == CARD_FLIPPING_UP)
        {
            c->flipT += FLIP_SPEED * dt;
            if (c->flipT >= 1.0f) { c->flipT = 1.0f; c->state = CARD_FACE_UP; }
        }
        else if (c->state == CARD_FLIPPING_DOWN)
        {
            c->flipT -= FLIP_SPEED * dt;
            if (c->flipT <= 0.0f) { c->flipT = 0.0f; c->state = CARD_FACE_DOWN; }
        }
    }

    // Decay flash timers
    if (mgState.wrongFlashT   > 0) mgState.wrongFlashT   -= dt;
    if (mgState.correctFlashT > 0) mgState.correctFlashT -= dt;

    switch (mgState.phase)
    {
        // ── Show all cards for a moment ──────────────────────────────
        case MG_PHASE_SHOW:
            mgState.phaseTimer -= dt;
            if (mgState.phaseTimer <= 0.0f)
            {
                // Flip all face-down
                for (int i = 0; i < mgState.cardCount; i++)
                    StartFlipDown(i);
                mgState.phase      = MG_PHASE_HIDE;
                mgState.phaseTimer = 0.7f; // wait for animation
            }
            break;

        case MG_PHASE_HIDE:
            mgState.phaseTimer -= dt;
            if (mgState.phaseTimer <= 0.0f)
                mgState.phase = MG_PHASE_PLAY;
            break;

        // ── Player input ─────────────────────────────────────────────
        case MG_PHASE_PLAY:
        {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                Vector2 mp = GetMousePosition();
                for (int i = 0; i < mgState.cardCount; i++)
                {
                    Card *c = &mgState.cards[i];
                    if (c->state != CARD_FACE_DOWN) continue;

                    Rectangle r = {c->x - c->width*0.5f,
                                   c->y - c->height*0.5f,
                                   c->width, c->height};
                    if (CheckCollisionPointRec(mp, r))
                    {
                        // Flip it up
                        StartFlipUp(i);
                        c->selected = true;
                        if (mgState.selCount < 2)
                            mgState.sel[mgState.selCount++] = i;

                        if (mgState.selCount == 2)
                        {
                            mgState.phase      = MG_PHASE_CHECK;
                            mgState.phaseTimer = 0.9f;
                        }
                        break;
                    }
                }
            }
        } break;

        // ── Brief pause after two cards selected ─────────────────────
        case MG_PHASE_CHECK:
            mgState.phaseTimer -= dt;
            if (mgState.phaseTimer <= 0.0f)
            {
                int a = mgState.sel[0];
                int b = mgState.sel[1];

                if (mgState.cards[a].symbol == mgState.cards[b].symbol)
                {
                    // Match!
                    mgState.cards[a].state    = CARD_MATCHED;
                    mgState.cards[b].state    = CARD_MATCHED;
                    mgState.cards[a].selected = false;
                    mgState.cards[b].selected = false;
                    mgState.pairsFound++;
                    globalScore += 100;
                    mgState.correctFlashT = 0.4f;
                    mgState.wrongStreak   = 0;
                    ctx->mood = EQUAL_NORMAL;
                }
                else
                {
                    // No match – flip back down
                    mgState.cards[a].selected = false;
                    mgState.cards[b].selected = false;
                    StartFlipDown(a);
                    StartFlipDown(b);
                    mgState.wrongFlashT  = 0.4f;
                    mgState.wrongStreak++;

                    // Equal gets angrier
                    if (mgState.wrongStreak >= 3)
                    {
                        ctx->mood      = EQUAL_GLITCH;
                        ctx->moodTimer = 0.6f;
                    }
                    else
                    {
                        ctx->mood      = EQUAL_ANGRY;
                        ctx->moodTimer = 0.5f;
                    }
                }

                mgState.sel[0]   = -1;
                mgState.sel[1]   = -1;
                mgState.selCount = 0;

                // Check end conditions
                bool allMatched = (mgState.pairsFound == mgState.pairsTotal);
                bool enoughFound = (mgState.pairsFound >= mgState.pairsNeeded);

                if (allMatched || enoughFound)
                {
                    mgState.passed     = true;
                    mgState.phase      = MG_PHASE_RESULT;
                    mgState.phaseTimer = 2.0f;
                    globalScore        += 500;
                    ctx->mood = EQUAL_NORMAL;
                }
                else
                {
                    // Check if any face-down unmatched cards remain
                    int remaining = 0;
                    for (int i = 0; i < mgState.cardCount; i++)
                        if (mgState.cards[i].state == CARD_FACE_DOWN)
                            remaining++;

                    if (remaining == 0)
                    {
                        // No more moves but didn't meet threshold → fail
                        mgState.passed     = false;
                        mgState.phase      = MG_PHASE_RESULT;
                        mgState.phaseTimer = 2.5f;
                        ctx->mood      = EQUAL_GLITCH;
                        ctx->moodTimer = 2.5f;
                    }
                    else
                    {
                        mgState.phase = MG_PHASE_PLAY;
                    }
                }
            }
            break;

        // ── Show result briefly then hand off ─────────────────────────
        case MG_PHASE_RESULT:
            mgState.phaseTimer -= dt;
            if (mgState.phaseTimer <= 0.0f)
            {
                mgState.phase = MG_PHASE_DONE;

                if (mgState.passed)
                {
                    ctx->cycle++;
                    ctx->state      = STATE_TRANSITION;
                    ctx->nextState  = STATE_MINIGAME_MEMORY; // loop for now
                    ctx->transTimer = 2.0f;
                }
                else
                {
                    ctx->lives--;
                    if (ctx->lives <= 0)
                        ctx->state = STATE_GAME_OVER;
                    else
                    {
                        // Retry same cycle
                        MemoryGame_Init(ctx);
                    }
                }
            }
            break;

        case MG_PHASE_DONE: break;
    }
}

// ── Draw helpers ─────────────────────────────────────────────────────────

static void DrawCard(Card *c, bool showFace, float flashCorrect, float flashWrong)
{
    // flipT: 0 = back, 1 = face.  Use cosine to simulate perspective squish.
    float scaleX = fabsf(cosf((1.0f - c->flipT) * 3.14159f * 0.5f));
    if (c->state == CARD_FACE_DOWN)  scaleX = 1.0f - scaleX; // reverse
    float hw = c->width  * 0.5f * scaleX;
    float hh = c->height * 0.5f;

    Rectangle r = {c->x - hw, c->y - hh, hw * 2.0f, hh * 2.0f};

    bool faceVisible = (c->flipT > 0.5f) ||
                       (c->state == CARD_FACE_UP) ||
                       (c->state == CARD_MATCHED);

    if (c->state == CARD_MATCHED)
    {
        // Matched card: green glow
        DrawRectangleRec(r, CLITERAL(Color){20,80,40,200});
        DrawRectangleLinesEx(r, 2.0f, COL_CORRECT);
        DrawText(SYMBOL_LABELS[c->symbol],
                 (int)(c->x - MeasureText(SYMBOL_LABELS[c->symbol], 20) * 0.5f),
                 (int)(c->y - 10), 20, COL_CORRECT);
        return;
    }

    if (faceVisible && showFace)
    {
        // Face-up
        Color bg = COL_CARD_FACE;
        if (c->selected && flashWrong > 0)
            bg = ColorAlpha(COL_WRONG, 0.6f + flashWrong * 0.4f);
        else if (c->selected && flashCorrect > 0)
            bg = ColorAlpha(COL_CORRECT, 0.6f + flashCorrect * 0.4f);

        DrawRectangleRec(r, bg);
        DrawRectangleLinesEx(r, 2.5f,
            c->selected ? COL_ACCENT : COL_DIM);

        // Symbol
        Color sc = SYMBOL_COLORS[c->symbol];
        int fs = 22;
        int tw = MeasureText(SYMBOL_LABELS[c->symbol], fs);
        DrawText(SYMBOL_LABELS[c->symbol],
                 (int)(c->x - tw * 0.5f), (int)(c->y - fs * 0.5f),
                 fs, sc);
    }
    else
    {
        // Face-down
        DrawRectangleRec(r, COL_CARD_BACK);
        DrawRectangleLinesEx(r, 2.0f, COL_ACCENT);

        // Pattern on back
        DrawLine((int)r.x, (int)r.y,
                 (int)(r.x + r.width), (int)(r.y + r.height),
                 ColorAlpha(COL_ACCENT, 0.25f));
        DrawLine((int)(r.x + r.width), (int)r.y,
                 (int)r.x, (int)(r.y + r.height),
                 ColorAlpha(COL_ACCENT, 0.25f));
        DrawText("?", (int)(c->x - 6), (int)(c->y - 11), 22, COL_DIM);
    }
}

// ── Draw ─────────────────────────────────────────────────────────────────
void MemoryGame_Draw(GameCtx *ctx)
{
    // Subtle scanline-ish background
    for (int y = 0; y < SCREEN_H; y += 4)
        DrawLine(0, y, SCREEN_W, y, ColorAlpha(COL_PANEL, 0.18f));

    // HUD
    DrawText(TextFormat("VIDAS: %d", ctx->lives),
             35, 75, 16, COL_EVIL);
    DrawText(TextFormat("PARES: %d / %d  (PRECISA DE %d)",
                        mgState.pairsFound,
                        mgState.pairsTotal,
                        mgState.pairsNeeded),
             180, 75, 16, COL_TEXT);

    // Phase label
    const char *phaseLabel = "";
    Color phaseColor = COL_TEXT;
    switch (mgState.phase)
    {
        case MG_PHASE_SHOW:
            phaseLabel = "Memorize!";
            phaseColor = COL_ACCENT;
            break;
        case MG_PHASE_HIDE:
        case MG_PHASE_PLAY:
            phaseLabel = "Encontre os pares!";
            break;
        case MG_PHASE_CHECK:
            phaseLabel = "...";
            break;
        case MG_PHASE_RESULT:
            if (mgState.passed)
            { phaseLabel = "PASSOU!";  phaseColor = COL_CORRECT; }
            else
            { phaseLabel = "ERROU!";   phaseColor = COL_EVIL; }
            break;
        default: break;
    }
    DrawText(phaseLabel,
             SCREEN_W / 2 - MeasureText(phaseLabel, 26) / 2,
             SCREEN_H - 36, 26, phaseColor);

    // Wrong flash overlay
    if (mgState.wrongFlashT > 0)
    {
        float a = mgState.wrongFlashT * 0.4f;
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H,
                      ColorAlpha(COL_EVIL, a));
    }
    if (mgState.correctFlashT > 0)
    {
        float a = mgState.correctFlashT * 0.35f;
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H,
                      ColorAlpha(COL_CORRECT, a));
    }

    // Cards
    bool showFace = (mgState.phase == MG_PHASE_SHOW) ||
                    (mgState.phase == MG_PHASE_HIDE);
    for (int i = 0; i < mgState.cardCount; i++)
    {
        bool sf = showFace ||
                  (mgState.cards[i].state == CARD_FACE_UP) ||
                  (mgState.cards[i].state == CARD_FLIPPING_UP) ||
                  (mgState.cards[i].state == CARD_MATCHED);
        DrawCard(&mgState.cards[i], sf,
                 mgState.correctFlashT, mgState.wrongFlashT);
    }

    // Equal on the right
    Equal_Draw(ctx, SCREEN_W * 0.83f, SCREEN_H * 0.48f, 1.0f);

    // "ERRADO" text when wrong (like concept art)
    if (mgState.wrongFlashT > 0)
    {
        float t = mgState.wrongFlashT;
        for (int i = 0; i < 4; i++)
        {
            int ex = (int)(SCREEN_W * 0.55f + (i % 2) * 70 - 35 +
                           (rand() % 6 - 3));
            int ey = (int)(SCREEN_H * 0.68f + (i / 2) * 30 +
                           (rand() % 6 - 3));
            DrawText("ERRADO", ex, ey, 22,
                     ColorAlpha(COL_EVIL, t * 1.5f > 1.0f ? 1.0f : t * 1.5f));
        }
    }
}