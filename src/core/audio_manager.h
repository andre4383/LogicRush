#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include "screens.h"

// Define phase-specific music IDs
typedef enum {
    MUSIC_NONE = -1,
    MUSIC_MENU = 0,
    MUSIC_QUIZ = 1,
    MUSIC_LABIRINTO = 2,
    MUSIC_BOSS = 3,
    MUSIC_COUNT = 4
} MusicID;

void AudioManager_Init(void);
void AudioManager_Update(float dt);
void AudioManager_TransitionToScreen(GameScreen screen);
void AudioManager_Unload(void);

#endif // AUDIO_MANAGER_H
