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
    "P=Q",   // SYM_EQUIV
    "P->Q",  // SYM_IMPL
    "P<->Q", // SYM_BICONDIT
    "!P v Q",// SYM_XOR (De Morgan)
    "!(P^Q)",// SYM_NAND
    "!PvQ",  // SYM_NOR
};

// Symbol colours for extra flair
static const Color SYMBOL_COLORS[SYM_COUNT] = {
    {  0, 255, 255, 255},   // bright cyan
    { 100, 200, 255, 255},   // sky blue
    { 220, 130, 255, 255},   // magenta-violet
    { 255, 215,  80, 255},   // amber
    { 255, 130, 130, 255},   // pink-red
    { 130, 255, 160, 255},   // mint
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
    mgState.phase             = MG_PHASE_SHOW;
    mgState.passed            = false;
    mgState.wrongStreak       = 0;
    mgState.correctStreak     = 0;
    mgState.comboTimer        = 0.0f;
    mgState.comboFeedbackTimer = 0.0f;

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
    if (mgState.wrongFlashT      > 0) mgState.wrongFlashT      -= dt;
    if (mgState.correctFlashT    > 0) mgState.correctFlashT    -= dt;
    if (ctx->hurtTimer           > 0) ctx->hurtTimer           -= dt;
    if (mgState.comboFeedbackTimer > 0) mgState.comboFeedbackTimer -= dt;

    // Combo window timeout — break streak if player was too slow
    if (mgState.comboTimer > 0) {
        mgState.comboTimer -= dt;
        if (mgState.comboTimer <= 0.0f) {
            mgState.comboTimer    = 0.0f;
            mgState.correctStreak = 0;
        }
    }

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
                    // Equal takes damage when player scores a pair!
                    ctx->mood       = EQUAL_HURT;
                    ctx->moodTimer  = 0.6f;
                    ctx->hurtTimer  = 0.6f;
                    // Combo streak tracking — 3 correct pairs within 8s = +1 vida
                    mgState.correctStreak++;
                    mgState.comboTimer = 8.0f;
                    if (mgState.correctStreak >= 3) {
                        if (ctx->lives < 3) {
                            ctx->lives++;
                            globalLives = ctx->lives;
                            mgState.comboFeedbackTimer = 1.8f;
                        }
                        mgState.correctStreak = 0;
                        mgState.comboTimer    = 0.0f;
                    }
                }
                else
                {
                    // No match – flip back down
                    mgState.cards[a].selected = false;
                    mgState.cards[b].selected = false;
                    StartFlipDown(a);
                    StartFlipDown(b);
                    mgState.wrongFlashT   = 0.4f;
                    mgState.wrongStreak++;
                    mgState.correctStreak = 0;
                    mgState.comboTimer    = 0.0f;

                    // Equal gets angrier (NOT damaged) when player misses
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
                    ctx->hurtTimer = 0.0f;  // ensure not showing hurt
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
                    globalLives = ctx->lives;  // sync global
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

    if (c->state == CARD_MATCHED)  // matched: electric effect
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
        // Face-up: dark chip body
        Color bg = COL_CARD_FACE;
        bool isMatched = (c->state == CARD_MATCHED);

        if (c->selected && flashWrong > 0) {
            bg = ColorAlpha(COL_WRONG, 0.5f + flashWrong * 0.5f);
        } else if (isMatched) {
            // BURNING chip: shows "burn" animation - golden core, dark edges
            float burnPulse = 0.5f + sinf((float)GetTime()*9.0f)*0.4f;
            bg = (Color){
                (unsigned char)(60 + 100*burnPulse),
                (unsigned char)(20 + 40*burnPulse),
                10, 255};
        } else if (c->selected && flashCorrect > 0) {
            // brief bright cyan flash on match
            bg = ColorAlpha(COL_CORRECT, 0.4f + flashCorrect * 0.5f);
        }

        DrawRectangleRec(r, bg);
        DrawRectangleLinesEx(r, 2.5f,
            isMatched ? (Color){255,150,40,255} :
            (c->selected ? COL_ACCENT : COL_DIM));

        // CHIP pins (small rectangles on sides)
        Color pinCol = isMatched ? (Color){100,40,20,255} : COL_DIM;
        for (int i = 0; i < 3; i++) {
            float py = r.y + r.height*(0.25f + i*0.25f) - 3.0f;
            DrawRectangle((int)(r.x-4),(int)py,4,6, pinCol);
            DrawRectangle((int)(r.x+r.width),(int)py,4,6, pinCol);
        }

        // BURN sparks on matched chips
        if (isMatched) {
            float t = (float)GetTime();
            for (int i = 0; i < 4; i++) {
                float sx = r.x + r.width*0.5f + sinf(t*5.0f+i*1.5f)*r.width*0.35f;
                float sy = r.y + r.height*0.5f + cosf(t*7.0f+i*1.8f)*r.height*0.3f;
                float ss = 1.5f + sinf(t*15.0f+i)*1.0f;
                DrawCircle((int)sx,(int)sy,(int)ss,
                           ColorAlpha((Color){255,180,60,255}, 0.7f));
            }
            // burn cracks
            DrawLineEx((Vector2){r.x+r.width*0.2f,r.y+r.height*0.3f},
                       (Vector2){r.x+r.width*0.7f,r.y+r.height*0.7f},
                       2.0f, ColorAlpha((Color){80,20,5,255},0.8f));
        }

        // Symbol — use bright/strong text color, NOT yellow on white
        Color sc = isMatched ? (Color){255,220,150,255} : SYMBOL_COLORS[c->symbol];
        int fs = 18;
        int tw = MeasureText(SYMBOL_LABELS[c->symbol], fs);
        // small shadow for legibility
        DrawText(SYMBOL_LABELS[c->symbol],
                 (int)(c->x - tw * 0.5f + 1), (int)(c->y - fs * 0.5f + 1),
                 fs, ColorAlpha(BLACK, 0.6f));
        DrawText(SYMBOL_LABELS[c->symbol],
                 (int)(c->x - tw * 0.5f), (int)(c->y - fs * 0.5f),
                 fs, sc);
    }
    else
    {
        // Face-down
        DrawRectangleRec(r, COL_CARD_BACK);
        DrawRectangleLinesEx(r, 2.0f, COL_ACCENT);

        // Chip-style pattern on back
        float cx = c->x, cy = c->y;
        float cw = c->width, ch = c->height;
        // Circuit lines
        DrawLine((int)(cx-cw*0.3f),(int)cy,(int)(cx+cw*0.3f),(int)cy,
                 ColorAlpha(COL_ACCENT,0.4f));
        DrawLine((int)cx,(int)(cy-ch*0.3f),(int)cx,(int)(cy+ch*0.3f),
                 ColorAlpha(COL_ACCENT,0.4f));
        DrawCircle((int)cx,(int)cy,5,ColorAlpha(COL_ACCENT,0.6f));
        DrawRectangleLines((int)(cx-12),(int)(cy-8),24,16,
                           ColorAlpha(COL_ACCENT,0.3f));
        DrawText("?", (int)(c->x - 6), (int)(c->y - 11), 22, COL_DIM);
    }
}

// ── Draw ─────────────────────────────────────────────────────────────────
void MemoryGame_Draw(GameCtx *ctx)
{
    // Subtle scanline-ish background
    for (int y = 0; y < SCREEN_H; y += 4)
        DrawLine(0, y, SCREEN_W, y, ColorAlpha(COL_PANEL, 0.18f));

    // Pairs info below top HUD
    DrawText(TextFormat("PARES: %d / %d  (PRECISA DE %d)",
                        mgState.pairsFound,
                        mgState.pairsTotal,
                        mgState.pairsNeeded),
             35, 75, 16, COL_TEXT);

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
             SCREEN_H - 76, 26, phaseColor);

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

    // Combo life-gain feedback
    if (mgState.comboFeedbackTimer > 0) {
        float a = mgState.comboFeedbackTimer > 0.5f ? 1.0f : mgState.comboFeedbackTimer * 2.0f;
        const char *comboMsg = "+1 VIDA!  COMBO!";
        int cw = MeasureText(comboMsg, 36);
        DrawText(comboMsg,
                 SCREEN_W / 2 - cw / 2,
                 SCREEN_H / 2 - 80,
                 36, ColorAlpha(COL_CORRECT, a));
    }

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