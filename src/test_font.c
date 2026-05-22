#include "raylib.h"
#include <stdio.h>

int main(void) {
    InitWindow(100, 100, "Font Test");
    SetTargetFPS(60);

    printf("Attempting to load font with LoadFontEx (size 96, NULL codepoints)...\n");
    Font f1 = LoadFontEx("assets/fonts/Inter.ttf", 96, NULL, 0);
    if (f1.texture.id == 0) {
        printf("FAILED to load f1!\n");
    } else {
        printf("SUCCESS to load f1! Texture ID: %u\n", f1.texture.id);
        UnloadFont(f1);
    }

    printf("Attempting to load font with simple LoadFont...\n");
    Font f2 = LoadFont("assets/fonts/Inter.ttf");
    if (f2.texture.id == 0) {
        printf("FAILED to load f2!\n");
    } else {
        printf("SUCCESS to load f2! Texture ID: %u\n", f2.texture.id);
        UnloadFont(f2);
    }

    CloseWindow();
    return 0;
}
