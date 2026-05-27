/*
 * Logic Rush — equal.c
 * Fase 1: ciborg animado com ciborg_spritesheet.png
 *
 * Layout do spritesheet (1400×1600, 2 colunas × 3 linhas):
 *
 *   col 0            col 1
 *  ┌─────────────┬─────────────┐  row 0
 *  │ Frame 0     │ Frame 1     │   idle / preparando
 *  │ (esquerda)  │ (direita)   │
 *  ├─────────────┼─────────────┤  row 1
 *  │ Frame 2     │ Frame 3     │   ataque roxo / tomando dano
 *  │ (esq. meio) │ (dir. meio) │
 *  ├─────────────┼─────────────┤  row 2
 *  │ Frame 4     │  (vazio)    │   derrota
 *  │ (esq. baixo)│             │
 *  └─────────────┴─────────────┘
 *
 * Mapeamento de estado → frame:
 *   EQUAL_NORMAL    → 0  (idle, nenhuma carta virada)
 *   EQUAL_PREPARING → 1  (carta virada, preparando ataque)
 *   EQUAL_ANGRY     → 2  (rajada roxa – 1-2 erros)
 *   EQUAL_GLITCH    → 2  (rajada roxa – 3+ erros)
 *   EQUAL_HURT      → 3  (tomando dano – par acertado → volta ao frame 1)
 *   EQUAL_DEFEATED  → 4  (derrota – player venceu)
 */

#include "raylib.h"
#include "game.h"
#include "../core/theme.h"
#include <math.h>
#include <stdio.h>

// ── Spritesheet ──────────────────────────────────────────────────────────────
static Texture2D texCiborg      = {0};
static bool      texCiborgLoaded = false;

#define CBX_COLS 2
#define CBX_ROWS 3

static void EnsureCiborgTex(void) {
    if (!texCiborgLoaded) {
        texCiborg = LoadTexture("assets/images/ciborg_spritesheet.png");
        texCiborgLoaded = (texCiborg.id > 0);
    }
}

// ── Mood update ──────────────────────────────────────────────────────────────
void Equal_UpdateMood(GameCtx *ctx, float dt) {
    if (ctx->moodTimer > 0.0f) {
        ctx->moodTimer -= dt;
        // Shake: maior quando glitch
        float amp  = (ctx->mood == EQUAL_GLITCH) ? 6.0f : 3.0f;
        ctx->shakeX = ((float)GetRandomValue(-100, 100) / 100.0f) * amp;
        ctx->shakeY = ((float)GetRandomValue(-100, 100) / 100.0f) * amp;

        if (ctx->moodTimer <= 0.0f) {
            // Após HURT/ANGRY/GLITCH volta para PREPARING (jogo ainda ativo)
            // PREPARING é o estado base enquanto o jogo de memória está em curso
            ctx->mood   = EQUAL_PREPARING;
            ctx->shakeX = 0.0f;
            ctx->shakeY = 0.0f;
        }
    }
}

// ── Draw ─────────────────────────────────────────────────────────────────────
void Equal_Draw(GameCtx *ctx, float x, float y, float scale) {
    EnsureCiborgTex();

    float t    = (float)GetTime();
    float bob  = ctx->shakeY + sinf(t * 2.2f) * 4.0f * scale;
    float shkX = ctx->shakeX;

    // ── Seleção de frame ─────────────────────────────────────────────────────
    // Frame index no grid 2×3:
    //   0 = (col0,row0) idle
    //   1 = (col1,row0) preparando ataque
    //   2 = (col0,row1) rajada roxa
    //   3 = (col1,row1) tomando dano
    //   4 = (col0,row2) derrota
    int frame;
    switch (ctx->mood) {
        case EQUAL_PREPARING: frame = 1; break;
        case EQUAL_ANGRY:
        case EQUAL_GLITCH:    frame = 2; break;
        case EQUAL_HURT:      frame = 3; break;
        case EQUAL_DEFEATED:  frame = 4; break;
        case EQUAL_NORMAL:
        default:              frame = 0; break;
    }

    if (texCiborgLoaded) {
        float fw  = roundf((float)texCiborg.width  / CBX_COLS);  // ~700 px
        float fh  = roundf((float)texCiborg.height / CBX_ROWS);  // ~533 px
        int   col = frame % CBX_COLS;
        int   row = frame / CBX_COLS;
        float ins = 1.0f;
        Rectangle src = { col*fw + ins, row*fh + ins, fw - ins*2, fh - ins*2 };

        float dH = 280.0f * scale;
        float dW = (fw > 0.0f) ? dH * (fw / fh) : dH;
        Rectangle dst = { x - dW*0.5f + shkX, y - dH*0.5f + bob, dW, dH };

        // ── Tint baseado no estado ────────────────────────────────────────────
        Color tint = WHITE;

        if (ctx->hurtTimer > 0.0f) {
            // Flash branco quando tomando dano (par acertado)
            float fa = ctx->hurtTimer / 0.6f;
            unsigned char wb = (unsigned char)(200 + (int)(55.0f * fa));
            tint = (Color){wb, wb, wb, 255};
        } else if (ctx->mood == EQUAL_ANGRY || ctx->mood == EQUAL_GLITCH) {
            // Tint roxo pulsante (rajada roxa)
            float p = 0.75f + sinf(t * 8.0f) * 0.25f;
            tint = (Color){(unsigned char)(180*p), 80, 255, 255};
        } else if (ctx->mood == EQUAL_DEFEATED) {
            // Levemente escurecido na derrota
            tint = (Color){200, 200, 200, 255};
        }

        DrawTexturePro(texCiborg, src, dst, (Vector2){0, 0}, 0.0f, tint);

        // ── Overlay de raios roxos (ANGRY / GLITCH) ───────────────────────────
        if (ctx->mood == EQUAL_ANGRY || ctx->mood == EQUAL_GLITCH) {
            Vector2 center = { dst.x + dW * 0.5f, dst.y + dH * 0.38f };
            for (int i = 0; i < 4; i++) {
                float ang = t * 2.5f + i * 1.5708f;   // 4 raios em 90°
                float r1  = dH * 0.26f;
                float r2  = r1 + 20.0f + sinf(t * 18.0f + i) * 9.0f;
                float flk = 0.5f + sinf(t * 28.0f + i * 1.4f) * 0.45f;
                DrawLineEx(
                    (Vector2){ center.x + cosf(ang)*r1, center.y + sinf(ang)*r1 },
                    (Vector2){ center.x + cosf(ang)*r2, center.y + sinf(ang)*r2 },
                    2.5f, ColorAlpha(COLOR_NEON_PURPLE, flk));
            }
        }

        // ── Scan-line de glitch (GLITCH) ──────────────────────────────────────
        if (ctx->mood == EQUAL_GLITCH && (int)(t * 20) % 3 == 0) {
            DrawRectangle(
                (int)(x - 50 + shkX),
                (int)(y + bob - 60 + GetRandomValue(-20, 20)),
                (int)(100 * scale), (int)(3 * scale),
                ColorAlpha(COLOR_NEON_PURPLE, 0.35f));
        }

        // ── Brilho de vitória (DEFEATED) ──────────────────────────────────────
        if (ctx->mood == EQUAL_DEFEATED) {
            float glow = 0.12f + sinf(t * 3.0f) * 0.06f;
            DrawRectangleLinesEx(dst, 3.0f, ColorAlpha(COLOR_NEON_GREEN, glow * 3.0f));
        }

    } else {
        // Fallback caso textura não carregue
        float dH = 200.0f*scale, dW = 120.0f*scale;
        DrawRectangle((int)(x-dW*0.5f+shkX), (int)(y-dH*0.5f+bob),
                      (int)dW, (int)dH, ColorAlpha(COLOR_NEON_PURPLE, 0.1f));
        DrawRectangleLinesEx(
            (Rectangle){x-dW*0.5f+shkX, y-dH*0.5f+bob, dW, dH},
            2.0f, ColorAlpha(COLOR_NEON_PURPLE, 0.5f));
        DrawText("CIBORG",
                 (int)(x - MeasureText("CIBORG", 14) * 0.5f),
                 (int)(y - 7), 14, COLOR_NEON_PURPLE);
    }
}

void Equal_Unload(void) {
    if (texCiborgLoaded) {
        UnloadTexture(texCiborg);
        texCiborgLoaded = false;
    }
}
