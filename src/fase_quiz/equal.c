/*
 * Logic Rush — equal.c
 * Phase 1 Equal AI character — drawn using the boss sprite sheet.
 */
#include "raylib.h"
#include "game.h"
#include "../core/theme.h"
#include <math.h>
#include <stdio.h>

// ── Sprite sheet (same file as fase_boss) ────────────────────────────────
static Texture2D texEqQuiz      = {0};
static bool      texEqQuizLoaded = false;
#define EQZ_COLS 3
#define EQZ_ROWS 4

static void EnsureEqTex(void) {
    if (!texEqQuizLoaded) {
        texEqQuiz = LoadTexture("assets/images/equal_spritesheet.png");
        if (texEqQuiz.id == 0)
            texEqQuiz = LoadTexture("assets/images/equal_spritesheet.jpg");
        texEqQuizLoaded = (texEqQuiz.id > 0);
    }
}

// ── Mood update ───────────────────────────────────────────────────────────
void Equal_UpdateMood(GameCtx *ctx, float dt) {
    if (ctx->moodTimer > 0.0f) {
        ctx->moodTimer -= dt;
        float amp = (ctx->mood == EQUAL_GLITCH) ? 6.0f : 3.0f;
        ctx->shakeX = ((float)(GetRandomValue(-100,100)) / 100.0f) * amp;
        ctx->shakeY = ((float)(GetRandomValue(-100,100)) / 100.0f) * amp;
        if (ctx->moodTimer <= 0.0f) {
            ctx->mood   = EQUAL_NORMAL;
            ctx->shakeX = 0.0f;
            ctx->shakeY = 0.0f;
        }
    }
}

// ── Draw Equal using sprite sheet ────────────────────────────────────────
void Equal_Draw(GameCtx *ctx, float x, float y, float scale) {
    EnsureEqTex();

    float t    = (float)GetTime();
    float bob  = ctx->shakeY + sinf(t * 2.2f) * 4.0f * scale;
    float shkX = ctx->shakeX;

    // Select frame
    // Row 0: normal — frames 0(idle),1(cast),2(attack)
    // Row 1: hurt/aggro — frames 3(follow),4(hurt),5(aggro)
    // Row 2: damaged — frames 6,7,8
    // Row 3: fury/death — frames 9,10,11
    int frame = 0;
    if (ctx->mood == EQUAL_GLITCH)  frame = 5;   // aggro
    else if (ctx->mood == EQUAL_ANGRY)  frame = 5;
    // no WORRIED mood in enum // hurt
    else frame = ((int)(t * 1.2f)) % 2;  // idle toggle 0/1

    if (texEqQuizLoaded) {
        float fw = roundf((float)texEqQuiz.width  / EQZ_COLS);
        float fh = roundf((float)texEqQuiz.height / EQZ_ROWS);
        int col  = frame % EQZ_COLS;
        int row  = frame / EQZ_COLS;
        float ins = 1.0f;
        Rectangle src = { col*fw+ins, row*fh+ins, fw-ins*2, fh-ins*2 };

        float dH = 260.0f * scale;
        float dW = (fw > 0) ? dH * (fw / fh) : dH;
        Rectangle dst = { x - dW*0.5f + shkX, y - dH*0.5f + bob, dW, dH };

        Color tint = WHITE;
        if (ctx->hurtTimer > 0.0f) {
            // White flash when hit
            float fa = ctx->hurtTimer / 0.6f;
            unsigned char wb = (unsigned char)(200 + (int)(55.0f * fa));
            tint = (Color){wb, wb, wb, 255};
        } else if (ctx->mood == EQUAL_GLITCH || ctx->mood == EQUAL_ANGRY) {
            float p = 0.8f + sinf(t*8.0f)*0.2f;
            tint = (Color){255,(unsigned char)(160*p),(unsigned char)(160*p),255};
        }

        DrawTexturePro(texEqQuiz, src, dst, (Vector2){0,0}, 0.0f, tint);

        // Rage lightning overlay
        if (ctx->mood == EQUAL_GLITCH || ctx->mood == EQUAL_ANGRY) {
            for (int i = 0; i < 3; i++) {
                float ang = t*3.0f + i*2.094f;
                float r1  = dH*0.28f;
                float r2  = r1 + 18.0f + sinf(t*20.0f+i)*8.0f;
                float flk = 0.5f + sinf(t*30.0f+i*1.3f)*0.4f;
                DrawLineEx(
                    (Vector2){dst.x+dW*0.5f+cosf(ang)*r1, dst.y+dH*0.38f+sinf(ang)*r1},
                    (Vector2){dst.x+dW*0.5f+cosf(ang)*r2, dst.y+dH*0.38f+sinf(ang)*r2},
                    2.5f, ColorAlpha(COLOR_NEON_RED, flk));
            }
        }
    } else {
        // Fallback rect
        float dH=200.0f*scale, dW=120.0f*scale;
        DrawRectangle((int)(x-dW*0.5f+shkX),(int)(y-dH*0.5f+bob),
                      (int)dW,(int)dH, ColorAlpha(COLOR_NEON_RED,0.1f));
        DrawRectangleLinesEx((Rectangle){x-dW*0.5f+shkX,y-dH*0.5f+bob,dW,dH},
                             2.f, ColorAlpha(COLOR_NEON_RED,0.5f));
        DrawText("EQUAL",(int)(x-MeasureText("EQUAL",14)*0.5f),(int)(y-7),
                 14, COLOR_NEON_RED);
    }

    // Glitch scan-line artifact
    if (ctx->mood == EQUAL_GLITCH && (int)(t*20)%3==0) {
        DrawRectangle((int)(x-50+shkX),(int)(y+bob-60+GetRandomValue(-20,20)),
                      (int)(100*scale),(int)(3*scale),
                      ColorAlpha(COLOR_NEON_RED,0.3f));
    }
}

void Equal_Unload(void) {
    if (texEqQuizLoaded) { UnloadTexture(texEqQuiz); texEqQuizLoaded=false; }
}
