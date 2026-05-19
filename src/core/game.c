#include "game.h"
#include "screens.h"

// Define the global active screen
GameScreen currentScreen = SCREEN_TITLE;

void InitGame(void) {
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
}
