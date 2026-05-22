/*
 * Logic Rush — equal.c
 * Draws the AI character "Equal" as a pixel-art style figure using
 * primitive shapes.  Two main moods: NORMAL (cyan) and ANGRY/GLITCH (red).
 */

#include "raylib.h"
#include "game.h"
#include <math.h>
#include <stdlib.h>

// ── Internal helpers ─────────────────────────────────────────────────────

static void DrawPixelRect(float cx, float cy, float w, float h,
                          Color fill, Color outline)
{
    DrawRectangle((int)(cx - w*0.5f), (int)(cy - h*0.5f),
                  (int)w, (int)h, outline);
    DrawRectangle((int)(cx - w*0.5f + 2), (int)(cy - h*0.5f + 2),
                  (int)(w - 4), (int)(h - 4), fill);
}

// ── Mood update ──────────────────────────────────────────────────────────

void Equal_UpdateMood(GameCtx *ctx, float dt)
{
    if (ctx->moodTimer > 0.0f)
    {
        ctx->moodTimer -= dt;

        // Shake intensity depends on mood
        float amp = (ctx->mood == EQUAL_GLITCH) ? 6.0f : 3.0f;
        ctx->shakeX = RandFloat(-amp, amp);
        ctx->shakeY = RandFloat(-amp, amp);

        if (ctx->moodTimer <= 0.0f)
        {
            ctx->moodTimer = 0.0f;
            ctx->shakeX    = 0.0f;
            ctx->shakeY    = 0.0f;
        }
    }
    else
    {
        ctx->shakeX = 0.0f;
        ctx->shakeY = 0.0f;
    }
}

// ── Draw Equal ──────────────────────────────────────────────────────────
/*
 * Draws a simplified pixel-art robot girl:
 *   - Round head with antenna "horns"
 *   - Body with tech patterns
 *   - Normal: cyan eyes + calm mouth
 *   - Angry : red eyes + jagged mouth
 *   - Glitch : angry + random colour artifacts
 */
void Equal_Draw(GameCtx *ctx, float x, float y, float scale)
{
    float ox = x + ctx->shakeX;
    float oy = y + ctx->shakeY;

    bool isEvil = (ctx->mood != EQUAL_NORMAL);

    Color bodyCol  = isEvil ? CLITERAL(Color){60,20,20,255}
                            : CLITERAL(Color){30,35,60,255};
    Color rimCol   = isEvil ? COL_EVIL : COL_ACCENT;
    Color eyeCol   = isEvil ? COL_EVIL : COL_ACCENT;
    Color skinCol  = CLITERAL(Color){200,210,230,255};

    float s = scale;

    // ── Left horn ──────────────────────────────────────────────────────
    {
        Vector2 pts[3] = {
            {ox - 22*s, oy - 38*s},
            {ox - 36*s, oy - 58*s},
            {ox - 10*s, oy - 52*s}
        };
        DrawTriangle(pts[0], pts[1], pts[2], rimCol);
    }
    // ── Right horn ─────────────────────────────────────────────────────
    {
        Vector2 pts[3] = {
            {ox + 22*s, oy - 38*s},
            {ox + 36*s, oy - 58*s},
            {ox + 10*s, oy - 52*s}
        };
        DrawTriangle(pts[2], pts[1], pts[0], rimCol);
    }

    // ── Head (circle) ──────────────────────────────────────────────────
    DrawCircle((int)ox, (int)(oy - 28*s), 28*s, rimCol);
    DrawCircle((int)ox, (int)(oy - 28*s), 24*s, bodyCol);

    // ── Eyes ───────────────────────────────────────────────────────────
    if (!isEvil)
    {
        // Normal: square cyan eyes
        DrawRectangle((int)(ox - 14*s), (int)(oy - 36*s),
                      (int)(10*s), (int)(10*s), eyeCol);
        DrawRectangle((int)(ox + 4*s),  (int)(oy - 36*s),
                      (int)(10*s), (int)(10*s), eyeCol);
    }
    else
    {
        // Evil: angled triangular eyes (like image 2 right side)
        // Left eye
        Vector2 le[3] = {
            {ox - 18*s, oy - 28*s},
            {ox - 6*s,  oy - 28*s},
            {ox - 12*s, oy - 36*s}
        };
        DrawTriangle(le[0], le[2], le[1], eyeCol);

        // Right eye
        Vector2 re[3] = {
            {ox + 6*s,  oy - 28*s},
            {ox + 18*s, oy - 28*s},
            {ox + 12*s, oy - 36*s}
        };
        DrawTriangle(re[0], re[2], re[1], eyeCol);
    }

    // ── Mouth ──────────────────────────────────────────────────────────
    if (!isEvil)
    {
        // Calm line
        DrawLineEx((Vector2){ox - 10*s, oy - 18*s},
                   (Vector2){ox + 10*s, oy - 18*s},
                   2.0f*s, eyeCol);
    }
    else
    {
        // Jagged evil grin (W shape)
        float my = oy - 18*s;
        float mx = ox;
        float tw = 16*s, th = 6*s;
        float thick = 2.0f * s;
        DrawLineEx((Vector2){mx - tw,   my},
                   (Vector2){mx - tw/2, my + th}, thick, eyeCol);
        DrawLineEx((Vector2){mx - tw/2, my + th},
                   (Vector2){mx,        my},       thick, eyeCol);
        DrawLineEx((Vector2){mx,        my},
                   (Vector2){mx + tw/2, my + th},  thick, eyeCol);
        DrawLineEx((Vector2){mx + tw/2, my + th},
                   (Vector2){mx + tw,   my},        thick, eyeCol);
    }

    // ── Neck ───────────────────────────────────────────────────────────
    DrawRectangle((int)(ox - 6*s), (int)(oy - 3*s),
                  (int)(12*s), (int)(8*s), skinCol);

    // ── Body ───────────────────────────────────────────────────────────
    DrawPixelRect(ox, oy + 20*s, 42*s, 36*s, bodyCol, rimCol);

    // Tech lines on chest
    DrawLineEx((Vector2){ox - 12*s, oy + 8*s},
               (Vector2){ox + 12*s, oy + 8*s}, 1.5f*s, rimCol);
    DrawLineEx((Vector2){ox - 8*s,  oy + 14*s},
               (Vector2){ox + 8*s,  oy + 14*s}, 1.0f*s, rimCol);
    DrawCircle((int)ox, (int)(oy + 20*s), (int)(5*s), rimCol);
    DrawCircle((int)ox, (int)(oy + 20*s), (int)(3*s), bodyCol);

    // ── Arms ───────────────────────────────────────────────────────────
    float armSwing = isEvil ? sinf(GetTime() * 12.0f) * 4.0f * s : 0;
    // Left arm
    DrawRectangle((int)(ox - 30*s), (int)(oy + 5*s + armSwing),
                  (int)(8*s), (int)(28*s), bodyCol);
    DrawRectangleLines((int)(ox - 30*s), (int)(oy + 5*s + armSwing),
                       (int)(8*s), (int)(28*s), rimCol);
    // Right arm
    DrawRectangle((int)(ox + 22*s), (int)(oy + 5*s - armSwing),
                  (int)(8*s), (int)(28*s), bodyCol);
    DrawRectangleLines((int)(ox + 22*s), (int)(oy + 5*s - armSwing),
                       (int)(8*s), (int)(28*s), rimCol);

    // ── Legs ───────────────────────────────────────────────────────────
    DrawPixelRect(ox - 12*s, oy + 52*s, 14*s, 24*s, bodyCol, rimCol);
    DrawPixelRect(ox + 12*s, oy + 52*s, 14*s, 24*s, bodyCol, rimCol);

    // ── Glitch artefacts ───────────────────────────────────────────────
    if (ctx->mood == EQUAL_GLITCH)
    {
        for (int i = 0; i < 5; i++)
        {
            float gx = RandFloat(ox - 30*s, ox + 30*s);
            float gy = RandFloat(oy - 60*s, oy + 64*s);
            float gw = RandFloat(4*s, 20*s);
            float gh = RandFloat(2*s, 6*s);
            Color gc = {
                (unsigned char)GetRandomValue(0,255),
                (unsigned char)GetRandomValue(0,255),
                (unsigned char)GetRandomValue(0,255),
                180
            };
            DrawRectangle((int)gx, (int)gy, (int)gw, (int)gh, gc);
        }
    }
}