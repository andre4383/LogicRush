#ifndef SCREENS_H
#define SCREENS_H

typedef enum GameScreen {
    SCREEN_LOGO = 0,
    SCREEN_TITLE,
    SCREEN_QUIZ,       // Fase 1: Quiz (Memory)
    SCREEN_GAMEPLAY,   // Fase 2: Labirinto
    SCREEN_BOSS,       // Fase 3: Boss Fight
    SCREEN_RANKING,    // Placar de Lideres (pos-boss)
    SCREEN_ENDING
} GameScreen;

#endif // SCREENS_H
