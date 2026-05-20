#include "raylib.h"
#include "game.h"

int main(void) {
    // Initialize Raylib window and OpenGL context
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Logic Rush — Desafios de Logica Proposicional");
    
    // Set rendering target frame rate
    SetTargetFPS(60);
    
    // Initialize all game screens and state
    InitGame();
    
    // Main game loop (runs until ESC is pressed or close button is clicked)
    while (!WindowShouldClose()) {
        UpdateGame();
        DrawGame();
    }
    
    // Unload assets and cleanup screen states
    UnloadGame();
    
    // Close OpenGL window and context
    CloseWindow();
    
    return 0;
}
