/*
 * Logic Rush — ranking_screen.c
 * Animated top-10 leaderboard. Appears after boss victory.
 * Also available standalone via the menu.
 */
#include "raylib.h"
#include "../core/game.h"
#include "../core/input.h"
#include "../core/theme.h"
#include "../core/screens.h"
#include "../core/ranking.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MAX_VISIBLE     10
#define ENTRY_H         48.0f
#define ENTRY_GAP       6.0f
#define ANIM_DELAY      0.12f   // seconds between each row flying in
#define ANIM_SLIDE_T    0.35f   // slide-in duration per row

// Name-entry state (shown when player qualifies)
static char  nameInput[RANKING_NAME_LEN] = {0};
static int   nameLen      = 0;
static bool  nameSaved    = false;
static bool  nameError    = false;   // duplicate name
static float nameErrorTimer = 0.0f;
static int   insertedPos  = -1;      // position this run's score was inserted

// Animation state per row
static float rowAnim[RANKING_MAX_ENTRIES] = {0}; // 0=offscreen, 1=final position
static float rowDelay[RANKING_MAX_ENTRIES] = {0};
static float screenTime = 0.0f;
static bool  rowsAnimDone = false;

// Particle system for new entry
typedef struct { float x, y, vx, vy, life, maxLife; Color col; } Particle;
#define MAX_PARTICLES 60
static Particle particles[MAX_PARTICLES];
static int particleCount = 0;

static void SpawnParticles(float x, float y) {
    for (int i = 0; i < 12 && particleCount < MAX_PARTICLES; i++) {
        Particle *p = &particles[particleCount++];
        p->x = x; p->y = y;
        p->vx = (float)(GetRandomValue(-80,80));
        p->vy = (float)(GetRandomValue(-120,-20));
        p->life = p->maxLife = 0.6f + GetRandomValue(0,40)*0.01f;
        p->col = (i%3==0) ? COLOR_NEON_CYAN : (i%3==1) ? COLOR_NEON_GREEN : COLOR_NEON_GOLD;
    }
}

// Called when entering ranking screen
// pendingScore>0 means player just finished and may enter name
static int   pendingScore  = 0;
static float pendingTime   = 0.0f;
static int   pendingPhase  = 3;
static bool  showNameEntry = false;

void Ranking_ScreenEnter(int score, float time, int phase) {
    pendingScore   = score;
    pendingTime    = time;
    pendingPhase   = phase;
    showNameEntry  = (score > 0 && Ranking_QualifiesForTop10(score));
    nameSaved      = false;
    nameError      = false;
    nameLen        = 0;
    nameInput[0]   = '\0';
    insertedPos    = -1;
    screenTime     = 0.0f;
    rowsAnimDone   = false;
    particleCount  = 0;

    // Set up staggered animation
    for (int i = 0; i < RANKING_MAX_ENTRIES; i++) {
        rowAnim[i]  = 0.0f;
        rowDelay[i] = i * ANIM_DELAY;
    }
}

void UpdateRankingScreen(void) {
    float dt = GetFrameTime();
    screenTime += dt;
    if (nameErrorTimer > 0) nameErrorTimer -= dt;

    // Prevent stale pause state from blocking ESC on this screen
    gamePaused = false;

    // Animate rows flying in
    if (!rowsAnimDone) {
        bool allDone = true;
        for (int i = 0; i < g_leaderboard.count; i++) {
            if (rowDelay[i] > 0) {
                rowDelay[i] -= dt;
                allDone = false;
            } else if (rowAnim[i] < 1.0f) {
                rowAnim[i] += dt / ANIM_SLIDE_T;
                if (rowAnim[i] > 1.0f) rowAnim[i] = 1.0f;
                allDone = false;
            }
        }
        rowsAnimDone = allDone;
    }

    // Update particles
    for (int i = 0; i < particleCount; i++) {
        Particle *p = &particles[i];
        p->life -= dt;
        p->x += p->vx * dt;
        p->y += p->vy * dt;
        p->vy += 120.0f * dt;  // gravity
        if (p->life <= 0) {
            particles[i] = particles[--particleCount];
            i--;
        }
    }

    if (Input_PressedCancel()) {
        currentScreen = SCREEN_TITLE;
        return;
    }

    // Name entry — use IsKeyPressed instead of GetCharPressed to avoid macOS
    // text input system (NSResponder/IME) intercepting the ESC key
    if (showNameEntry && !nameSaved) {
        // Letters A-Z (stored uppercase; KEY_A..KEY_Z == ASCII 'A'..'Z')
        for (int k = KEY_A; k <= KEY_Z; k++) {
            if (IsKeyPressed(k) && nameLen < RANKING_NAME_LEN - 1) {
                nameInput[nameLen++] = (char)k;
                nameInput[nameLen]   = '\0';
            }
        }
        // Digits 0-9 (KEY_ZERO..KEY_NINE == ASCII '0'..'9')
        for (int k = KEY_ZERO; k <= KEY_NINE; k++) {
            if (IsKeyPressed(k) && nameLen < RANKING_NAME_LEN - 1) {
                nameInput[nameLen++] = (char)k;
                nameInput[nameLen]   = '\0';
            }
        }
        // Underscore: Shift+Minus on most layouts
        if (IsKeyPressed(KEY_MINUS)
            && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
            && nameLen < RANKING_NAME_LEN - 1) {
            nameInput[nameLen++] = '_';
            nameInput[nameLen]   = '\0';
        }

        if (IsKeyPressedRepeat(KEY_BACKSPACE) && nameLen > 0)
            nameInput[--nameLen] = '\0';

        if (IsKeyPressed(KEY_ENTER) && nameLen > 0) {
            if (Ranking_NameExists(nameInput)) {
                nameError      = true;
                nameErrorTimer = 2.0f;
            } else {
                insertedPos = Ranking_Add(nameInput, pendingScore, pendingTime, pendingPhase);
                nameSaved   = true;
                nameError   = false;
                showNameEntry = false;
                // Reset row animations so the new entry slides in
                for (int i = 0; i < RANKING_MAX_ENTRIES; i++) {
                    rowDelay[i] = i * ANIM_DELAY * 0.5f;
                    rowAnim[i]  = (i == insertedPos) ? 0.0f : 1.0f;
                }
                rowsAnimDone = false;
            }
        }
    }
}

void DrawRankingScreen(void) {
    float t = (float)GetTime();

    ClearBackground(COLOR_BG_DARK);
    DrawThemeGrid(SCREEN_WIDTH, SCREEN_HEIGHT, CELL_SIZE);

    // Scanline overlay
    for (int y = 0; y < SCREEN_HEIGHT; y += 3)
        DrawLine(0, y, SCREEN_WIDTH, y, ColorAlpha(BLACK, 0.08f));

    // Title
    const char *title = "HALL OF FAME";
    float pulse = 0.8f + sinf(t*2.5f)*0.18f;
    int tw = MeasureTextBold(title, 42);
    DrawTextBold(title, SCREEN_WIDTH/2 - tw/2 + 3, 28+3, 42,
                 ColorAlpha(COLOR_NEON_CYAN, 0.3f));
    DrawTextBold(title, SCREEN_WIDTH/2 - tw/2, 28, 42,
                 ColorAlpha(COLOR_NEON_CYAN, pulse));

    // Decorative line
    DrawLineEx((Vector2){60, 82}, (Vector2){SCREEN_WIDTH-60, 82},
               1.5f, ColorAlpha(COLOR_NEON_CYAN, 0.35f));

    // Column headers
    int hdrY = 96;
    DrawText("#",      90,  hdrY, 14, COLOR_TEXT_MUTED);
    DrawText("NOME",   160, hdrY, 14, COLOR_TEXT_MUTED);
    DrawText("PONTOS", 480, hdrY, 14, COLOR_TEXT_MUTED);
    DrawText("FASE",   680, hdrY, 14, COLOR_TEXT_MUTED);
    DrawText("TEMPO",  820, hdrY, 14, COLOR_TEXT_MUTED);
    DrawLine(80, hdrY+18, SCREEN_WIDTH-80, hdrY+18,
             ColorAlpha(COLOR_PANEL_BORDER, 0.5f));

    // Entries
    static const char *medals[3] = {"1ST", "2ND", "3RD"};
    static const Color medalCols[3] = {
        {234,179,8,255}, {192,192,192,255}, {205,127,50,255}
    };

    float listStartY = hdrY + 26.0f;

    for (int i = 0; i < g_leaderboard.count && i < MAX_VISIBLE; i++) {
        float anim = rowAnim[i];
        // Ease-out cubic
        float ease = 1.0f - (1.0f-anim)*(1.0f-anim)*(1.0f-anim);

        // Slide in from left
        float slideX = (1.0f - ease) * -(float)SCREEN_WIDTH;

        float rowY = listStartY + i * (ENTRY_H + ENTRY_GAP);
        Rectangle rowRect = { 70 + slideX, rowY, SCREEN_WIDTH - 140.0f, ENTRY_H };

        // Highlight new entry
        bool isNew = (i == insertedPos);
        float newPulse = isNew ? (0.5f + sinf(t*6.0f)*0.4f) : 0.0f;

        // Row background
        Color rowBg = isNew
            ? ColorAlpha(COLOR_NEON_CYAN, 0.12f + newPulse*0.08f)
            : (i%2==0 ? ColorAlpha(COLOR_PANEL_BG, 0.7f) : ColorAlpha(COLOR_PANEL_BG, 0.4f));
        DrawRectangleRec(rowRect, rowBg);
        Color rowBorder = isNew ? COLOR_NEON_CYAN
                        : ColorAlpha(COLOR_PANEL_BORDER, 0.4f);
        DrawRectangleLinesEx(rowRect, 1.0f, rowBorder);

        RankEntry *e = &g_leaderboard.entries[i];
        float tx = rowRect.x;
        float ty = rowY + ENTRY_H/2 - 9;

        // Medal / rank number
        if (i < 3) {
            DrawTextBold(medals[i], (int)(tx+12), (int)ty, 14, medalCols[i]);
        } else {
            char rankStr[8]; sprintf(rankStr, "%d", i+1);
            DrawText(rankStr, (int)(tx+18), (int)ty, 14, COLOR_TEXT_MUTED);
        }

        // Name
        Color nameCol = isNew ? COLOR_NEON_CYAN
                      : (i==0 ? COLOR_NEON_GOLD : i==1 ? (Color){192,192,192,255}
                               : i==2 ? (Color){205,127,50,255} : COLOR_TEXT_MAIN);
        DrawTextBold(e->name, (int)(tx+78), (int)ty, 16, nameCol);

        // Score
        char sc[16]; sprintf(sc, "%d", e->score);
        DrawTextBold(sc, (int)(tx+400), (int)ty, 16, COLOR_NEON_GREEN);

        // Phase
        char ph[8]; sprintf(ph, "FASE %d", e->phase);
        DrawText(ph, (int)(tx+600), (int)ty, 13, COLOR_TEXT_CYBER);

        // Time
        char tm[16]; sprintf(tm, "%.0fs", e->time);
        DrawText(tm, (int)(tx+740), (int)ty, 13, COLOR_TEXT_MUTED);

        // Spawn particles on new entry appearing
        if (isNew && anim > 0.0f && anim < 0.05f && particleCount < 10)
            SpawnParticles(rowRect.x + rowRect.width*0.5f, rowY + ENTRY_H*0.5f);
    }

    // Draw particles
    for (int i = 0; i < particleCount; i++) {
        Particle *p = &particles[i];
        float alpha = p->life / p->maxLife;
        DrawCircleV((Vector2){p->x,p->y}, 4.0f*alpha+1.0f,
                    ColorAlpha(p->col, alpha));
    }

    // Empty state
    if (g_leaderboard.count == 0) {
        const char *empty = "Nenhum score registrado ainda. Seja o primeiro!";
        DrawText(empty, SCREEN_WIDTH/2 - MeasureText(empty,16)/2,
                 SCREEN_HEIGHT/2, 16, COLOR_TEXT_MUTED);
    }

    // Name entry panel
    if (showNameEntry && !nameSaved) {
        // Semi-transparent overlay
        DrawRectangle(0, SCREEN_HEIGHT-190, SCREEN_WIDTH, 190,
                      ColorAlpha(COLOR_BG_DARK, 0.92f));
        DrawLineEx((Vector2){0,(float)(SCREEN_HEIGHT-190)},
                   (Vector2){(float)SCREEN_WIDTH,(float)(SCREEN_HEIGHT-190)},
                   1.5f, ColorAlpha(COLOR_NEON_CYAN, 0.5f));

        char qualMsg[64];
        sprintf(qualMsg, "PARABENS! Voce entrou no TOP 10 com %d pontos!", pendingScore);
        int qw = MeasureText(qualMsg, 16);
        DrawText(qualMsg, SCREEN_WIDTH/2-qw/2, SCREEN_HEIGHT-172, 16, COLOR_NEON_GOLD);

        const char *prompt = "Digite seu nome (A-Z, 0-9) e pressione ENTER:";
        int pw = MeasureText(prompt, 14);
        DrawText(prompt, SCREEN_WIDTH/2-pw/2, SCREEN_HEIGHT-146, 14, COLOR_TEXT_MUTED);

        // Input box
        Rectangle inputBox = {SCREEN_WIDTH/2-160.0f, SCREEN_HEIGHT-120.0f, 320.0f, 42.0f};
        bool blink = (int)(t*2) % 2 == 0;
        DrawRectangleRec(inputBox, ColorAlpha(COLOR_PANEL_BG, 0.95f));
        DrawRectangleLinesEx(inputBox, 2.0f,
            nameError ? COLOR_NEON_RED : ColorAlpha(COLOR_NEON_CYAN, 0.8f));
        char disp[RANKING_NAME_LEN+2];
        sprintf(disp, "%s%s", nameInput, blink ? "_" : " ");
        int dw = MeasureTextBold(disp, 20);
        DrawTextBold(disp, (int)(inputBox.x + inputBox.width/2 - dw/2),
                     (int)(inputBox.y + 10), 20, COLOR_NEON_CYAN);

        // Error message
        if (nameError && nameErrorTimer > 0) {
            const char *errMsg = "Este nome ja esta em uso! Escolha outro.";
            int ew = MeasureText(errMsg, 13);
            DrawText(errMsg, SCREEN_WIDTH/2-ew/2, SCREEN_HEIGHT-68,
                     13, COLOR_NEON_RED);
        }

        // Hint
        DrawText("ESC: Voltar ao Menu  |  BACKSPACE: Apagar",
                 SCREEN_WIDTH/2 - MeasureText("ESC: Voltar ao Menu  |  BACKSPACE: Apagar",12)/2,
                 SCREEN_HEIGHT-20, 12, ColorAlpha(COLOR_TEXT_MUTED, 0.6f));
    } else if (!showNameEntry) {
        // Navigation hint
        const char *hint = nameSaved
            ? "Score salvo!  ESC: Voltar ao Menu"
            : "ESC: Voltar ao Menu";
        DrawText(hint, SCREEN_WIDTH/2 - MeasureText(hint,13)/2,
                 SCREEN_HEIGHT-22, 13, ColorAlpha(COLOR_TEXT_MUTED, 0.7f));
    }

    DrawThemeVignette(SCREEN_WIDTH, SCREEN_HEIGHT);
}
