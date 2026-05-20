#include "game.h"
#include "screens.h"

// Define the global active screen
GameScreen currentScreen = SCREEN_TITLE;

void InitGame(void) {
    InitTitleScreen();
    InitGameplayScreen();
    InitBossScreen();
}

void UpdateGame(void) {
    switch (currentScreen) {
        case SCREEN_TITLE:
            UpdateTitleScreen();
            break;
        case SCREEN_GAMEPLAY:
            UpdateGameplayScreen();
            break;
        case SCREEN_BOSS:
            UpdateBossScreen();
            break;
        default:
            break;
    }
}

void DrawGame(void) {
    BeginDrawing();

    switch (currentScreen) {
        case SCREEN_TITLE:
            DrawTitleScreen();
            break;
        case SCREEN_GAMEPLAY:
            DrawGameplayScreen();
            break;
        case SCREEN_BOSS:
            DrawBossScreen();
            break;
        default:
            ClearBackground(DARKGRAY);
            DrawText("Tela desconhecida!", 20, 20, 20, RED);
            break;
    }

    EndDrawing();
}

void UnloadGame(void) {
    UnloadTitleScreen();
    UnloadGameplayScreen();
    UnloadBossScreen();
}
