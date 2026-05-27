/*
 * Logic Rush — Ranking Viewer (Executavel Separado)
 *
 * Programa standalone que mostra o placar do Logic Rush em tempo real.
 * Detecta mudancas no arquivo logicRushRanking.dat e atualiza automaticamente.
 * Ideal para rodar em monitor secundario durante eventos.
 */
#include "raylib.h"
#include "../core/ranking.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define WIN_W   900
#define WIN_H   720
#define ENTRY_H 50.0f
#define ENTRY_GAP 6.0f

// Color palette (matches main game)
#define COL_BG          (Color){ 10, 15, 26, 255 }
#define COL_PANEL_BG    (Color){ 13, 20, 35, 255 }
#define COL_PANEL_BORDER (Color){ 20, 30, 50, 255 }
#define COL_GRID        (Color){ 59, 130, 246, 255 }
#define COL_CYAN        (Color){ 34, 211, 238, 255 }
#define COL_GREEN       (Color){ 34, 197, 94, 255 }
#define COL_GOLD        (Color){ 234, 179, 8, 255 }
#define COL_RED         (Color){ 239, 68, 68, 255 }
#define COL_TEXT        (Color){ 241, 245, 249, 255 }
#define COL_MUTED       (Color){ 148, 163, 184, 255 }

static Font fontMain, fontBold;

// Animation state per row (for new entries)
static float rowAnim[RANKING_MAX_ENTRIES] = {0};
static int   newEntryRow = -1;
static float newEntryHighlight = 0.0f;
static long lastModTime = 0;
static float reloadFlash = 0.0f;

static int HashName(const char *s) {
    int h = 0;
    while (*s) { h = h*31 + (unsigned char)*s; s++; }
    return h;
}

static bool LoadRankingFromDisk(void) {
    Leaderboard old = g_leaderboard;
    g_leaderboard.count = 0;
    FILE *f = fopen(RANKING_FILE, "rb");
    if (!f) return false;
    int n = 0;
    if (fread(&n, sizeof(int), 1, f) == 1 && n > 0 && n <= RANKING_MAX_ENTRIES) {
        g_leaderboard.count = n;
        for (int i = 0; i < n; i++)
            fread(&g_leaderboard.entries[i], sizeof(RankEntry), 1, f);
    }
    fclose(f);

    // Detect new entries
    newEntryRow = -1;
    for (int i = 0; i < g_leaderboard.count; i++) {
        int h = HashName(g_leaderboard.entries[i].name);
        bool found = false;
        for (int j = 0; j < old.count; j++) {
            if (HashName(old.entries[j].name) == h) { found = true; break; }
        }
        if (!found) {
            newEntryRow = i;
            newEntryHighlight = 3.0f;
            // Reset animation for that row
            rowAnim[i] = 0.0f;
            break;
        }
    }
    return true;
}

static long FileModTime(const char *path) {
    return GetFileModTime(path);
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(WIN_W, WIN_H, "Logic Rush - Placar de Lideres");
    SetExitKey(KEY_ESCAPE);
    SetTargetFPS(60);

    fontMain = LoadFontEx("assets/fonts/JetBrainsMono-Regular.ttf", 64, NULL, 0);
    fontBold = LoadFontEx("assets/fonts/JetBrainsMono-Bold.ttf", 64, NULL, 0);
    if (fontMain.texture.id == 0) fontMain = GetFontDefault();
    if (fontBold.texture.id == 0) fontBold = fontMain;
    SetTextureFilter(fontMain.texture, TEXTURE_FILTER_TRILINEAR);
    SetTextureFilter(fontBold.texture, TEXTURE_FILTER_TRILINEAR);

    // Initial load
    LoadRankingFromDisk();
    lastModTime = FileModTime(RANKING_FILE);
    for (int i = 0; i < RANKING_MAX_ENTRIES; i++) rowAnim[i] = 1.0f;

    float pollTimer = 0.5f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        float t  = (float)GetTime();
        int sw = GetScreenWidth(), sh = GetScreenHeight();

        // Poll file every 0.5s
        pollTimer -= dt;
        if (pollTimer <= 0.0f) {
            pollTimer = 0.5f;
            long mt = FileModTime(RANKING_FILE);
            if (mt != lastModTime) {
                lastModTime = mt;
                LoadRankingFromDisk();
                reloadFlash = 1.0f;
            }
        }

#ifdef __APPLE__
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_F)) ToggleFullscreen();
#else
        if (IsKeyPressed(KEY_F11)) ToggleFullscreen();
#endif

        // Animation step
        for (int i = 0; i < g_leaderboard.count; i++) {
            if (rowAnim[i] < 1.0f) {
                rowAnim[i] += dt / 0.4f;
                if (rowAnim[i] > 1.0f) rowAnim[i] = 1.0f;
            }
        }
        if (newEntryHighlight > 0) newEntryHighlight -= dt;
        if (reloadFlash > 0) reloadFlash -= dt;

        BeginDrawing();
        ClearBackground(COL_BG);

        // Background grid
        for (int x = 0; x < sw; x += 40)
            DrawLine(x, 0, x, sh, ColorAlpha(COL_GRID, 0.04f));
        for (int y = 0; y < sh; y += 40)
            DrawLine(0, y, sw, y, ColorAlpha(COL_GRID, 0.04f));

        // Title
        const char *title = "LOGIC RUSH";
        const char *subtitle = "Placar de Lideres";
        float titleW = MeasureTextEx(fontBold, title, 56, 1.0f).x;
        float subW   = MeasureTextEx(fontMain, subtitle, 22, 1.0f).x;
        float pulse = 0.85f + sinf(t*2.5f)*0.15f;
        DrawTextEx(fontBold, title, (Vector2){sw*0.5f-titleW*0.5f+3,28+3}, 56, 1.0f,
                   ColorAlpha(COL_CYAN, 0.3f));
        DrawTextEx(fontBold, title, (Vector2){sw*0.5f-titleW*0.5f,28}, 56, 1.0f,
                   ColorAlpha(COL_CYAN, pulse));
        DrawTextEx(fontMain, subtitle, (Vector2){sw*0.5f-subW*0.5f,90}, 22, 1.0f, COL_MUTED);

        // Decorative line
        DrawLineEx((Vector2){80, 130}, (Vector2){sw-80, 130}, 1.5f,
                   ColorAlpha(COL_CYAN, 0.5f));

        // Status/reload indicator
        if (reloadFlash > 0) {
            const char *upd = "ATUALIZADO!";
            float w = MeasureTextEx(fontBold, upd, 18, 1.0f).x;
            DrawTextEx(fontBold, upd, (Vector2){sw-w-30, 30}, 18, 1.0f,
                       ColorAlpha(COL_GREEN, reloadFlash));
        }
        // "Live" indicator
        DrawCircle(30, 38, 5+sinf(t*3.0f)*2, ColorAlpha(COL_GREEN, 0.8f));
        DrawTextEx(fontMain, "AO VIVO", (Vector2){45,32}, 14, 1.0f, COL_GREEN);

        // Column headers
        float listX = 80.0f;
        float listY = 160.0f;
        float listW = sw - 160.0f;
        DrawTextEx(fontMain, "#",      (Vector2){listX+10, listY}, 14, 1.0f, COL_MUTED);
        DrawTextEx(fontMain, "NOME",   (Vector2){listX+75, listY}, 14, 1.0f, COL_MUTED);
        DrawTextEx(fontMain, "PONTOS", (Vector2){listX+350, listY}, 14, 1.0f, COL_MUTED);
        DrawTextEx(fontMain, "FASE",   (Vector2){listX+490, listY}, 14, 1.0f, COL_MUTED);
        DrawTextEx(fontMain, "TEMPO",  (Vector2){listX+600, listY}, 14, 1.0f, COL_MUTED);
        DrawLine((int)listX, (int)listY+22, (int)(listX+listW), (int)listY+22,
                 ColorAlpha(COL_PANEL_BORDER, 0.7f));

        static const char *medals[3] = {"1ST", "2ND", "3RD"};
        static const Color medalCols[3] = {
            {234,179,8,255}, {192,192,192,255}, {205,127,50,255}
        };

        // Entries
        float entY = listY + 32.0f;
        for (int i = 0; i < g_leaderboard.count; i++) {
            float ease = 1.0f - (1.0f-rowAnim[i])*(1.0f-rowAnim[i])*(1.0f-rowAnim[i]);
            float slideX = (1.0f - ease) * -sw;

            float rowY = entY + i * (ENTRY_H + ENTRY_GAP);
            Rectangle rowRect = { listX + slideX, rowY, listW, ENTRY_H };

            bool isNew = (i == newEntryRow && newEntryHighlight > 0);
            float newP = isNew ? (0.4f + sinf(t*6.0f)*0.4f) : 0.0f;

            Color rowBg = isNew
                ? ColorAlpha(COL_CYAN, 0.18f + newP*0.10f)
                : (i%2==0 ? ColorAlpha(COL_PANEL_BG, 0.85f) : ColorAlpha(COL_PANEL_BG, 0.5f));
            DrawRectangleRec(rowRect, rowBg);
            Color rowBorder = isNew ? COL_CYAN
                            : ColorAlpha(COL_PANEL_BORDER, 0.5f);
            DrawRectangleLinesEx(rowRect, isNew ? 2.0f : 1.0f, rowBorder);

            RankEntry *e = &g_leaderboard.entries[i];
            float tx = rowRect.x;
            float ty = rowY + ENTRY_H*0.5f - 10.0f;

            if (i < 3) {
                DrawTextEx(fontBold, medals[i], (Vector2){tx+12, ty}, 18, 1.0f, medalCols[i]);
            } else {
                char rs[8]; sprintf(rs, "%d", i+1);
                DrawTextEx(fontBold, rs, (Vector2){tx+18, ty}, 18, 1.0f, COL_MUTED);
            }
            Color nameCol = isNew ? COL_CYAN
                          : (i==0 ? COL_GOLD : i==1 ? (Color){192,192,192,255}
                                   : i==2 ? (Color){205,127,50,255} : COL_TEXT);
            DrawTextEx(fontBold, e->name, (Vector2){tx+75, ty}, 20, 1.0f, nameCol);

            char sc[16]; sprintf(sc, "%d", e->score);
            DrawTextEx(fontBold, sc, (Vector2){tx+350, ty}, 20, 1.0f, COL_GREEN);

            char ph[8]; sprintf(ph, "F%d", e->phase);
            DrawTextEx(fontMain, ph, (Vector2){tx+510, ty+2}, 16, 1.0f, COL_CYAN);

            char tm[16]; sprintf(tm, "%.0fs", e->time);
            DrawTextEx(fontMain, tm, (Vector2){tx+620, ty+2}, 16, 1.0f, COL_MUTED);
        }

        // Empty state
        if (g_leaderboard.count == 0) {
            const char *empty = "Aguardando o primeiro jogador...";
            float w = MeasureTextEx(fontMain, empty, 20, 1.0f).x;
            DrawTextEx(fontMain, empty, (Vector2){sw*0.5f-w*0.5f, sh*0.5f}, 20, 1.0f, COL_MUTED);
        }

        // Footer
        const char *footer = "ESC: Sair  |  F11: Tela Cheia  |  Atualizacao automatica";
        float fw = MeasureTextEx(fontMain, footer, 13, 1.0f).x;
        DrawTextEx(fontMain, footer, (Vector2){sw*0.5f-fw*0.5f, sh-28}, 13, 1.0f, COL_MUTED);

        EndDrawing();
    }

    UnloadFont(fontMain);
    UnloadFont(fontBold);
    CloseWindow();
    return 0;
}
