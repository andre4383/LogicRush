#include "audio_manager.h"
#include "raylib.h"
#include <stddef.h>

static Music musics[MUSIC_COUNT];
static bool musicLoaded[MUSIC_COUNT] = { false };
static int currentActiveMusic = MUSIC_NONE;
static int fadingOutMusic = MUSIC_NONE;

#define MAX_VOLUME 0.5f
static float currentActiveVolume = 0.0f;
static float fadingOutVolume = 0.0f;
static float fadeSpeed = 0.4f; // Duration: ~2.5s (longer transitions)

void AudioManager_Init(void) {
    InitAudioDevice();
    
    const char* menuPath = "assets/music/Fase Menu.mp3";
    const char* quizPathNoAccent = "assets/music/Fase jogo da memoria 1.wav";
    const char* quizPathNFC = "assets/music/Fase jogo da memória 1.wav";
    const char* quizPathNFD = "assets/music/Fase jogo da memo\xcc\x81ria 1.wav";
    const char* labirintoPath = "assets/music/Fase Labirinto 2.wav";
    const char* bossPath = "assets/music/Fase Boss 3.mp3";
    
    if (FileExists(menuPath)) {
        musics[MUSIC_MENU] = LoadMusicStream(menuPath);
        musics[MUSIC_MENU].looping = true;
        musicLoaded[MUSIC_MENU] = true;
    } else {
        TraceLog(LOG_WARNING, "AudioManager: Could not find Menu music!");
    }
    
    const char* quizPath = quizPathNoAccent;
    if (!FileExists(quizPath)) {
        quizPath = quizPathNFC;
    }
    if (!FileExists(quizPath)) {
        quizPath = quizPathNFD;
    }
    if (FileExists(quizPath)) {
        musics[MUSIC_QUIZ] = LoadMusicStream(quizPath);
        musics[MUSIC_QUIZ].looping = true;
        musicLoaded[MUSIC_QUIZ] = true;
    } else {
        TraceLog(LOG_WARNING, "AudioManager: Could not find Memory Game music!");
    }
    
    if (FileExists(labirintoPath)) {
        musics[MUSIC_LABIRINTO] = LoadMusicStream(labirintoPath);
        musics[MUSIC_LABIRINTO].looping = true;
        musicLoaded[MUSIC_LABIRINTO] = true;
    } else {
        TraceLog(LOG_WARNING, "AudioManager: Could not find Labyrinth music!");
    }
    
    if (FileExists(bossPath)) {
        musics[MUSIC_BOSS] = LoadMusicStream(bossPath);
        musics[MUSIC_BOSS].looping = true;
        musicLoaded[MUSIC_BOSS] = true;
    } else {
        TraceLog(LOG_WARNING, "AudioManager: Could not find Boss music!");
    }
}

void AudioManager_TransitionToScreen(GameScreen screen) {
    int targetMusic = MUSIC_NONE;
    switch (screen) {
        case SCREEN_TITLE:
        case SCREEN_RANKING:
            targetMusic = MUSIC_MENU;
            break;
        case SCREEN_QUIZ:
            targetMusic = MUSIC_QUIZ;
            break;
        case SCREEN_GAMEPLAY:
            targetMusic = MUSIC_LABIRINTO;
            break;
        case SCREEN_BOSS:
            targetMusic = MUSIC_BOSS;
            break;
        default:
            targetMusic = MUSIC_NONE;
            break;
    }
    
    if (targetMusic == currentActiveMusic) {
        return;
    }
    
    TraceLog(LOG_INFO, "AudioManager: Transitioning from %d to %d (screen: %d)", currentActiveMusic, targetMusic, screen);
    
    // Stop any existing fading out music immediately to avoid conflicts
    if (fadingOutMusic != MUSIC_NONE && musicLoaded[fadingOutMusic]) {
        TraceLog(LOG_INFO, "AudioManager: Stopping fading out music %d", fadingOutMusic);
        StopMusicStream(musics[fadingOutMusic]);
    }
    
    fadingOutMusic = currentActiveMusic;
    fadingOutVolume = currentActiveVolume;
    
    currentActiveMusic = targetMusic;
    if (currentActiveMusic != MUSIC_NONE && musicLoaded[currentActiveMusic]) {
        TraceLog(LOG_INFO, "AudioManager: Playing music %d", currentActiveMusic);
        PlayMusicStream(musics[currentActiveMusic]);
        SetMusicVolume(musics[currentActiveMusic], 0.0f);
        currentActiveVolume = 0.0f;
    } else {
        TraceLog(LOG_WARNING, "AudioManager: Music %d not loaded or MUSIC_NONE", currentActiveMusic);
        currentActiveVolume = 0.0f;
    }
}

void AudioManager_Update(float dt) {
    // Raylib streaming update requires calling UpdateMusicStream()
    if (fadingOutMusic != MUSIC_NONE && musicLoaded[fadingOutMusic]) {
        UpdateMusicStream(musics[fadingOutMusic]);
        fadingOutVolume -= fadeSpeed * dt;
        if (fadingOutVolume <= 0.0f) {
            fadingOutVolume = 0.0f;
            StopMusicStream(musics[fadingOutMusic]);
            fadingOutMusic = MUSIC_NONE;
        } else {
            SetMusicVolume(musics[fadingOutMusic], fadingOutVolume);
        }
    }
    
    if (currentActiveMusic != MUSIC_NONE && musicLoaded[currentActiveMusic]) {
        UpdateMusicStream(musics[currentActiveMusic]);
        if (currentActiveVolume < MAX_VOLUME) {
            currentActiveVolume += fadeSpeed * dt;
            if (currentActiveVolume > MAX_VOLUME) {
                currentActiveVolume = MAX_VOLUME;
            }
            SetMusicVolume(musics[currentActiveMusic], currentActiveVolume);
        }
    }
}

void AudioManager_Unload(void) {
    for (int i = 0; i < MUSIC_COUNT; i++) {
        if (musicLoaded[i]) {
            StopMusicStream(musics[i]);
            UnloadMusicStream(musics[i]);
            musicLoaded[i] = false;
        }
    }
    CloseAudioDevice();
}
