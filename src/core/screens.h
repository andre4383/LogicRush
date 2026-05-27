#ifndef SCREENS_H
#define SCREENS_H

typedef enum GameScreen {
    SCREEN_LOGO = 0,
    SCREEN_TITLE,
    SCREEN_INTRO,         // Sequência de introdução (Modo História)
    SCREEN_STORY_DIALOGUE,// Diálogos entre fases  (Modo História)
    SCREEN_QUIZ,          // Fase 1: Quiz (Memory)
    SCREEN_GAMEPLAY,      // Fase 2: Labirinto
    SCREEN_BOSS,          // Fase 3: Boss Fight
    SCREEN_RANKING,       // Placar de Lideres (pos-boss competitivo)
    SCREEN_ENDING
} GameScreen;

typedef enum GameMode {
    MODE_COMPETITIVE = 0, // Modo com pontuação e ranking (padrão)
    MODE_STORY            // Modo narrativo com diálogos
} GameMode;

#endif // SCREENS_H
