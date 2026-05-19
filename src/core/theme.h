#ifndef THEME_H
#define THEME_H

#include "raylib.h"
#include <math.h>

// ==================================================================================
//                              LOGIC RUSH DESIGN SYSTEM
// ==================================================================================
// Use these constants to keep color schemes and visual assets unified across screens.

// Core Palettes
#define COLOR_BG_DARK           (Color){ 10, 15, 26, 255 }     // Cyber Cadet deep blue-black
#define COLOR_PANEL_BG          (Color){ 13, 20, 35, 255 }     // Card background
#define COLOR_PANEL_BORDER      (Color){ 20, 30, 50, 255 }     // Default tech border
#define COLOR_GRID_LINE         (Color){ 59, 130, 246, 255 }   // Blue tech grid lines

// Neon Glow Colors
#define COLOR_NEON_CYAN         (Color){ 34, 211, 238, 255 }   // Laser/Wires/Trails
#define COLOR_NEON_GREEN        (Color){ 34, 197, 94, 255 }    // Active states/Player
#define COLOR_NEON_RED          (Color){ 239, 68, 68, 255 }    // Inactive states
#define COLOR_NEON_PURPLE       (Color){ 168, 85, 247, 255 }   // Gates/Portals
#define COLOR_NEON_MAGENTA      (Color){ 217, 70, 239, 255 }   // Gates/Portals (Alt)
#define COLOR_NEON_GOLD         (Color){ 234, 179, 8, 255 }    // OR Gate/Warning lines

// Typography / Text Colors
#define COLOR_TEXT_MAIN         (Color){ 241, 245, 249, 255 }  // High contrast white
#define COLOR_TEXT_MUTED        (Color){ 148, 163, 184, 255 }  // Muted gray-slate
#define COLOR_TEXT_CYBER        (Color){ 96, 165, 250, 255 }   // Tech highlight blue

// ==================================================================================
//                              THEME DRAWING HELPERS
// ==================================================================================

// 1. Draw standard background cyber grid
static inline void DrawThemeGrid(int width, int height, int cellSize) {
    for (int x = 0; x < width; x += cellSize) {
        Color col = (x % (cellSize * 4) == 0) 
            ? ColorAlpha(COLOR_GRID_LINE, 0.08f) 
            : ColorAlpha(COLOR_GRID_LINE, 0.03f);
        DrawLine(x, cellSize, x, height, col);
    }
    for (int y = cellSize; y < height; y += cellSize) {
        Color col = (y % (cellSize * 4) == 0) 
            ? ColorAlpha(COLOR_GRID_LINE, 0.08f) 
            : ColorAlpha(COLOR_GRID_LINE, 0.03f);
        DrawLine(0, y, width, y, col);
    }
}

// 2. Draw glassmorphism rounded panels
static inline void DrawThemeGlassPanel(Rectangle rect, float roundness, Color borderCol) {
    DrawRectangleRounded(rect, roundness, 4, ColorAlpha(COLOR_PANEL_BG, 0.85f));
    DrawRectangleRoundedLines(rect, roundness, 4, borderCol);
}

// 3. Draw ambient technical vignette (for screen corners)
static inline void DrawThemeVignette(int width, int height) {
    DrawRectangleGradientV(0, 0, width, 40, ColorAlpha(BLACK, 0.45f), ColorAlpha(BLACK, 0.0f));
    DrawRectangleGradientV(0, height - 40, width, 40, ColorAlpha(BLACK, 0.0f), ColorAlpha(BLACK, 0.45f));
    DrawRectangleGradientH(0, 0, 40, height, ColorAlpha(BLACK, 0.45f), ColorAlpha(BLACK, 0.0f));
    DrawRectangleGradientH(width - 40, 0, 40, height, ColorAlpha(BLACK, 0.0f), ColorAlpha(BLACK, 0.45f));
}

// 4. Draw glowing tech buttons with hover effects
static inline void DrawThemeButton(Rectangle rect, const char* text, int fontSize, bool hovered, Color activeColor) {
    Color bgCol = hovered ? ColorAlpha(activeColor, 0.15f) : ColorAlpha(COLOR_PANEL_BG, 0.6f);
    Color lineCol = hovered ? activeColor : COLOR_PANEL_BORDER;
    Color textCol = hovered ? WHITE : COLOR_TEXT_MUTED;
    
    DrawRectangleRounded(rect, 0.2f, 4, bgCol);
    DrawRectangleRoundedLines(rect, 0.2f, 4, lineCol);
    
    // Draw button text centered
    int textW = MeasureText(text, fontSize);
    DrawText(text, rect.x + rect.width / 2.0f - textW / 2.0f, rect.y + rect.height / 2.0f - fontSize / 2.0f, fontSize, textCol);
}

// 5. Draw interactive cyan laser beam between two points
static inline void DrawThemeLaser(Vector2 start, Vector2 end, float pulseTime) {
    Color beamColor = ColorAlpha(COLOR_NEON_CYAN, 0.45f + sinf(pulseTime * 15.0f) * 0.15f);
    DrawLineEx(start, end, 2.0f, beamColor);
    DrawCircleV(end, 16.0f + sinf(pulseTime * 12.0f) * 3.0f, ColorAlpha(COLOR_NEON_CYAN, 0.18f));
}

// 6. Draw circuit trace with right angles and moving electrons
static inline void DrawThemeCircuitWire(Vector2 start, Vector2 end, bool active, float timeVal) {
    Color col = active ? COLOR_NEON_GREEN : COLOR_TEXT_MUTED;
    Color glowCol = active ? ColorAlpha(COLOR_NEON_GREEN, 0.25f) : ColorAlpha(COLOR_TEXT_MUTED, 0.05f);
    Vector2 mid = { end.x, start.y };
    
    // Glow line (thicker)
    DrawLineEx(start, mid, 6.0f, glowCol);
    DrawLineEx(mid, end, 6.0f, glowCol);
    
    // Core line (thinner)
    DrawLineEx(start, mid, 2.5f, col);
    DrawLineEx(mid, end, 2.5f, col);
    
    // Flowing electrons
    if (active) {
        float speed = 120.0f; // pixels per second
        float segment1 = fabsf(mid.x - start.x);
        float segment2 = fabsf(end.y - mid.y);
        float totalLen = segment1 + segment2;
        
        if (totalLen > 0) {
            for (int e = 0; e < 2; e++) {
                float progress = fmodf(timeVal * speed + e * (totalLen / 2.0f), totalLen);
                Vector2 electronPos;
                
                if (progress < segment1) {
                    float t = progress / segment1;
                    electronPos.x = start.x + (mid.x - start.x) * t;
                    electronPos.y = start.y;
                } else {
                    float t = (progress - segment1) / segment2;
                    electronPos.x = mid.x;
                    electronPos.y = mid.y + (end.y - mid.y) * t;
                }
                
                DrawCircleV(electronPos, 3.5f, WHITE);
                DrawCircleV(electronPos, 6.0f, ColorAlpha(COLOR_NEON_GREEN, 0.6f));
            }
        }
    }
}

#endif // THEME_H
