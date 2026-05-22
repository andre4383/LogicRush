/*
 * Logic Rush — utils.c
 * Tiny helper functions used everywhere.
 */

#include "raylib.h"
#include "game.h"
#include <math.h>
#include <stdlib.h>

// ── Random float in [lo, hi] ─────────────────────────────────────────────
float RandFloat(float lo, float hi)
{
    return lo + (float)rand() / (float)RAND_MAX * (hi - lo);
}

// ── Draw text centred on X axis at a given Y ─────────────────────────────
void DrawTextCentered(Font font, const char *text, float y,
                      float fontSize, float spacing, Color tint)
{
    Vector2 size = MeasureTextEx(font, text, fontSize, spacing);
    DrawTextEx(font, text,
               (Vector2){(SCREEN_W - size.x) * 0.5f, y},
               fontSize, spacing, tint);
}