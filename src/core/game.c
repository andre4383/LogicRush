#include "game.h"
#include "screens.h"

// Define the global active screen
GameScreen currentScreen = SCREEN_TITLE;

// Global font variables
Font fontMain;
Font fontBold;

void InitGame(void) {
    // Create list of codepoints including Portuguese characters
    int codepoints[256];
    int count = 0;
    for (int i = 32; i < 127; i++) {
        codepoints[count++] = i;
    }
    int pt_chars[] = {
        0x00C1, 0x00E1, // Á, á
        0x00C0, 0x00E0, // À, à
        0x00C2, 0x00E2, // Â, â
        0x00C3, 0x00E3, // Ã, ã
        0x00C9, 0x00E9, // É, é
        0x00CA, 0x00EA, // Ê, ê
        0x00CD, 0x00ED, // Í, í
        0x00D3, 0x00F3, // Ó, ó
        0x00D4, 0x00F4, // Ô, ô
        0x00D5, 0x00F5, // Õ, õ
        0x00DA, 0x00FA, // Ú, ú
        0x00C7, 0x00E7  // Ç, ç
    };
    for (int i = 0; i < (int)(sizeof(pt_chars)/sizeof(pt_chars[0])); i++) {
        codepoints[count++] = pt_chars[i];
    }

    // Load custom fonts with high resolution (96px) for crisp rendering
    fontMain = LoadFontEx("assets/fonts/Inter.ttf", 96, codepoints, count);
    fontBold = LoadFontEx("assets/fonts/Inter-Bold.ttf", 96, codepoints, count);
    
    // Set texture filter to bilinear for smooth scaling
    SetTextureFilter(fontMain.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(fontBold.texture, TEXTURE_FILTER_BILINEAR);
    
    // Initialise all screens
    InitTitleScreen();
    InitGameplayScreen();
}

void UpdateGame(void) {
    switch (currentScreen) {
        case SCREEN_TITLE:
            UpdateTitleScreen();
            break;
        case SCREEN_GAMEPLAY:
            UpdateGameplayScreen();
            break;
        default:
            break;
    }
}

void DrawGame(void) {
    BeginDrawing();
    
    // Draw the active screen
    switch (currentScreen) {
        case SCREEN_TITLE:
            DrawTitleScreen();
            break;
        case SCREEN_GAMEPLAY:
            DrawGameplayScreen();
            break;
        default:
            ClearBackground(DARKGRAY);
            DrawText("Tela desconhecida!", 20, 20, 20, RED);
            break;
    }
    
    EndDrawing();
}

void UnloadGame(void) {
    // Clean up assets of all screens
    UnloadTitleScreen();
    UnloadGameplayScreen();
    
    // Unload global fonts
    UnloadFont(fontMain);
    UnloadFont(fontBold);
}
