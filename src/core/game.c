#include "game.h"
#include "screens.h"
#include "theme.h"
#include "../fase_quiz/game.h"
#include "../fase_quiz/memory_game.h"
#include <stdio.h>
#include "ranking.h"

// Define the global active screen
GameScreen currentScreen = SCREEN_TITLE;

// Global font variables
Font fontMain;
Font fontBold;

// Global persistent state across phases
float globalTimer = 0.0f;
int globalScore = 0;
bool gameRunning = false;
bool gamePaused = false;
int  globalLives = 3;   // shared across phases

// Phase Banner Transition Overlay System
float phaseBannerTimer = 0.0f;
const char* phaseBannerTitle = "";
const char* phaseBannerSubtitle = "";

void StartPhaseBanner(const char* title, const char* subtitle) {
    phaseBannerTimer = 3.0f;
    phaseBannerTitle = title;
    phaseBannerSubtitle = subtitle;
}

void UpdatePhaseBanner(float dt) {
    if (phaseBannerTimer > 0.0f) {
        phaseBannerTimer -= dt;
        if (phaseBannerTimer < 0.0f) {
            phaseBannerTimer = 0.0f;
        }
    }
}

void DrawPhaseBanner(void) {
    if (phaseBannerTimer <= 0.0f) return;
    
    float alpha = 1.0f;
    if (phaseBannerTimer > 2.5f) {
        alpha = (3.0f - phaseBannerTimer) / 0.5f;
    } else if (phaseBannerTimer < 0.8f) {
        alpha = phaseBannerTimer / 0.8f;
    }
    
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;
    
    // Draw full-screen blur/dim overlay
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha((Color){ 10, 12, 28, 255 }, alpha * 0.85f));
    
    // Draw horizontal tech banner in the center
    float bannerHeight = 160.0f;
    Rectangle bannerRect = { 0, SCREEN_HEIGHT / 2.0f - bannerHeight / 2.0f, SCREEN_WIDTH, bannerHeight };
    DrawRectangleRec(bannerRect, ColorAlpha(COLOR_PANEL_BG, alpha * 0.9f));
    DrawLineEx((Vector2){ 0, bannerRect.y }, (Vector2){ SCREEN_WIDTH, bannerRect.y }, 2.0f, ColorAlpha(COLOR_NEON_CYAN, alpha * 0.8f));
    DrawLineEx((Vector2){ 0, bannerRect.y + bannerHeight }, (Vector2){ SCREEN_WIDTH, bannerRect.y + bannerHeight }, 2.0f, ColorAlpha(COLOR_NEON_CYAN, alpha * 0.8f));
    
    // Glowing text in the middle
    int titleSize = 48;
    int subtitleSize = 20;
    
    int titleW = MeasureTextBold(phaseBannerTitle, titleSize);
    int subW = MeasureText(phaseBannerSubtitle, subtitleSize);
    
    // Glow effect
    Color glowCol = ColorAlpha(COLOR_NEON_CYAN, alpha * 0.35f);
    DrawTextBold(phaseBannerTitle, SCREEN_WIDTH / 2 - titleW / 2 + 2, SCREEN_HEIGHT / 2.0f - 40 + 2, titleSize, glowCol);
    
    // Main text
    DrawTextBold(phaseBannerTitle, SCREEN_WIDTH / 2 - titleW / 2, SCREEN_HEIGHT / 2.0f - 40, titleSize, ColorAlpha(COLOR_TEXT_MAIN, alpha));
    DrawText(phaseBannerSubtitle, SCREEN_WIDTH / 2 - subW / 2, SCREEN_HEIGHT / 2.0f + 18, subtitleSize, ColorAlpha(COLOR_TEXT_MUTED, alpha));
}

// Unified Glassmorphic HUD drawing helper
void DrawUnifiedHUD(const char* phaseTitle, const char* phaseSubtitle, const char* helpText) {
    // Glassmorphic top panel
    DrawThemeGlassPanel((Rectangle){ 15, 10, SCREEN_WIDTH - 30, 48 }, 0.20f, ColorAlpha(COLOR_GRID_LINE, 0.25f));

    // Left: Game name & active phase
    char titleText[128];
    if (phaseSubtitle && phaseSubtitle[0] != '\0') {
        sprintf(titleText, "LOGIC RUSH: %s — %s", phaseTitle, phaseSubtitle);
    } else {
        sprintf(titleText, "LOGIC RUSH: %s", phaseTitle);
    }
    DrawText(titleText, 35, 24, 18, COLOR_TEXT_CYBER);

    (void)helpText; // commands moved to DrawBottomHUD

    // Middle-Right: Timer
    char timerText[32];
    sprintf(timerText, "TEMPO: %.1f s", globalTimer);
    DrawText(timerText, SCREEN_WIDTH - 430, 24, 18, COLOR_TEXT_MAIN);

    // Right: Score
    char scoreText[32];
    sprintf(scoreText, "SCORE: %05d", globalScore);
    DrawText(scoreText, SCREEN_WIDTH - 250, 24, 18, COLOR_NEON_GOLD);

    // Far-right: Lives as hearts
    int hStartX = SCREEN_WIDTH - 110;
    int hY = 19;
    for (int i = 0; i < 3; i++) {
        Color hCol = (i < globalLives) ? COLOR_NEON_GREEN
                                       : ColorAlpha(COLOR_TEXT_MUTED, 0.22f);
        float hx = (float)(hStartX + i * 26);
        float hy = (float)hY;
        // Heart: two overlapping circles on top + downward triangle
        DrawCircle((int)(hx + 5),  (int)(hy + 5), 5.5f, hCol);
        DrawCircle((int)(hx + 13), (int)(hy + 5), 5.5f, hCol);
        DrawTriangle(
            (Vector2){hx - 1.0f,        hy + 7.0f},
            (Vector2){hx + 19.0f,       hy + 7.0f},
            (Vector2){hx + 9.0f,        hy + 19.0f},
            hCol);
        // Glow for active lives
        if (i < globalLives)
            DrawCircle((int)(hx + 9), (int)(hy + 9), 11.0f,
                       ColorAlpha(hCol, 0.10f));
    }
}

// Bottom HUD bar — commands/controls for the active phase
void DrawBottomHUD(const char* commands) {
    DrawThemeGlassPanel((Rectangle){ 15, SCREEN_HEIGHT - 35, SCREEN_WIDTH - 30, 28 },
                        0.20f, ColorAlpha(COLOR_GRID_LINE, 0.15f));
    if (commands) {
        int w = MeasureText(commands, 13);
        DrawText(commands, SCREEN_WIDTH / 2 - w / 2, SCREEN_HEIGHT - 28, 13, COLOR_TEXT_MUTED);
    }
}

// Quiz lifecycle wrappers
GameCtx quizCtx;

void InitQuizScreen(void) {
    quizCtx.state = STATE_MINIGAME_MEMORY;
    quizCtx.cycle = 0;
    quizCtx.lives = globalLives;
    quizCtx.mood = EQUAL_NORMAL;
    quizCtx.moodTimer = 0.0f;
    quizCtx.shakeX = 0.0f;
    quizCtx.shakeY = 0.0f;
    quizCtx.paused = false;
    MemoryGame_Init(&quizCtx);
}

void UpdateQuizScreen(void) {
    float dt = GetFrameTime();
    
    // Update global banner if active
    if (phaseBannerTimer > 0.0f) {
        UpdatePhaseBanner(dt);
        return; // Freeze gameplay update during intro banner
    }
    
    if (quizCtx.state != STATE_GAME_OVER) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            gamePaused = true;
            return;
        }
    }
    
    if (quizCtx.state == STATE_GAME_OVER) {
        Rectangle card = { SCREEN_WIDTH / 2.0f - 250, SCREEN_HEIGHT / 2.0f - 150, 500, 300 };
        Rectangle btnRestart = { SCREEN_WIDTH / 2.0f - 180, card.y + 180, 160, 45 };
        Rectangle btnMenu = { SCREEN_WIDTH / 2.0f + 20, card.y + 180, 160, 45 };
        Vector2 mousePos = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mousePos, btnRestart)) {
                globalScore = 0;
                globalTimer = 0.0f;
                InitQuizScreen();
            }
            if (CheckCollisionPointRec(mousePos, btnMenu)) {
                currentScreen = SCREEN_TITLE;
            }
        }
        if (IsKeyPressed(KEY_SPACE)) {
            globalScore = 0;
            globalTimer = 0.0f;
            InitQuizScreen();
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            currentScreen = SCREEN_TITLE;
        }
        return;
    }
    
    globalTimer += dt;
    MemoryGame_Update(&quizCtx, dt);
    Equal_UpdateMood(&quizCtx, dt);
    
    if (quizCtx.state == STATE_GAME_OVER) {
        return;
    }
    
    // Check if the memory game has transitioned to done/passed
    extern MemoryGameState mgState;
    if (mgState.phase == MG_PHASE_DONE) {
        if (mgState.passed) {
            // Quiz passed! Transition to Labyrinth (Fase 2)
            currentScreen = SCREEN_GAMEPLAY;
            InitGameplayScreen();
            StartPhaseBanner("FASE 2", "LABIRINTO DE PORTAS");
        } else {
            quizCtx.state = STATE_GAME_OVER;
            // Trigger ranking screen on death
            Ranking_ScreenEnter(globalScore, globalTimer, 1);
            currentScreen = SCREEN_RANKING;
        }
    }
}

void DrawQuizScreen(void) {
    ClearBackground(COLOR_BG_DARK);
    DrawThemeGrid(SCREEN_WIDTH, SCREEN_HEIGHT, CELL_SIZE);
    
    MemoryGame_Draw(&quizCtx);
    
    DrawUnifiedHUD("FASE 1: QUIZ", "EQUIVALÊNCIAS", NULL);
#ifdef __APPLE__
    DrawBottomHUD("ESC: Pausar  |  Clique nas cartas para combinar  |  Ctrl+F: Fullscreen");
#else
    DrawBottomHUD("ESC: Pausar  |  Clique nas cartas para combinar  |  F11: Fullscreen");
#endif
    
    // Draw Game Over overlay if needed
    if (quizCtx.state == STATE_GAME_OVER) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha((Color){ 15, 10, 10, 255 }, 0.90f));
        
        Rectangle card = { SCREEN_WIDTH / 2.0f - 250, SCREEN_HEIGHT / 2.0f - 150, 500, 300 };
        DrawThemeGlassPanel(card, 0.15f, COLOR_NEON_RED);
        
        const char* failText = "CONEXÃO CORROMPIDA!";
        int failTextW = MeasureText(failText, 30);
        DrawText(failText, SCREEN_WIDTH / 2 - failTextW / 2, card.y + 40, 30, COLOR_NEON_RED);
        
        const char* descText = "Você falhou em descriptografar os dados da Equal.";
        int descTextW = MeasureText(descText, 16);
        DrawText(descText, SCREEN_WIDTH / 2 - descTextW / 2, card.y + 90, 16, COLOR_TEXT_MUTED);
        
        char statsText[64];
        sprintf(statsText, "Tempo de Execução: %.1f s", globalTimer);
        int statsTextW = MeasureText(statsText, 16);
        DrawText(statsText, SCREEN_WIDTH / 2 - statsTextW / 2, card.y + 130, 16, COLOR_TEXT_MAIN);
        
        Rectangle btnRestart = { SCREEN_WIDTH / 2.0f - 180, card.y + 180, 160, 45 };
        Rectangle btnMenu = { SCREEN_WIDTH / 2.0f + 20, card.y + 180, 160, 45 };
        
        Vector2 mousePos = GetMousePosition();
        bool hoveredRestart = CheckCollisionPointRec(mousePos, btnRestart);
        bool hoveredMenu = CheckCollisionPointRec(mousePos, btnMenu);
        
        DrawThemeButton(btnRestart, "REINICIAR (ESP)", 12, hoveredRestart, COLOR_NEON_GREEN);
        DrawThemeButton(btnMenu, "MENU (ESC)", 12, hoveredMenu, COLOR_TEXT_MUTED);
    }
    
    // Draw Phase Banner overlay on top
    DrawPhaseBanner();
}

void UnloadQuizScreen(void) {
    MemoryGame_Free();
}

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
    fontMain = LoadFontEx("assets/fonts/JetBrainsMono-Regular.ttf", 96, codepoints, count);
    fontBold = LoadFontEx("assets/fonts/JetBrainsMono-Bold.ttf", 96, codepoints, count);
    
    // Generate mipmaps and set filter to TRILINEAR for super crisp downscaling
    GenTextureMipmaps(&fontMain.texture);
    GenTextureMipmaps(&fontBold.texture);
    SetTextureFilter(fontMain.texture, TEXTURE_FILTER_TRILINEAR);
    SetTextureFilter(fontBold.texture, TEXTURE_FILTER_TRILINEAR);
    
    // Load leaderboard from disk
    Ranking_Load();

    // Initialise all screens
    InitTitleScreen();
    InitGameplayScreen();
    InitBossScreen();
}

void DrawPauseOverlay(void) {
    if (!gamePaused) return;
    
    // Escurece o fundo
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha((Color){ 8, 10, 20, 255 }, 0.75f));
    
    // Painel principal
    Rectangle card = { SCREEN_WIDTH / 2.0f - 200, SCREEN_HEIGHT / 2.0f - 120, 400, 240 };
    DrawThemeGlassPanel(card, 0.15f, COLOR_NEON_CYAN);
    
    // Título
    const char* pauseTitle = "SISTEMA PAUSADO";
    int titleW = MeasureTextBold(pauseTitle, 24);
    DrawTextBold(pauseTitle, SCREEN_WIDTH / 2 - titleW / 2, card.y + 30, 24, COLOR_NEON_CYAN);
    
    // Descrição
    const char* pauseDesc = "A execução dos processos foi suspensa.";
    int descW = MeasureText(pauseDesc, 14);
    DrawText(pauseDesc, SCREEN_WIDTH / 2 - descW / 2, card.y + 65, 14, COLOR_TEXT_MUTED);
    
    // Botões
    Rectangle btnResume = { SCREEN_WIDTH / 2.0f - 150, card.y + 100, 300, 45 };
    Rectangle btnMenu = { SCREEN_WIDTH / 2.0f - 150, card.y + 160, 300, 45 };
    
    Vector2 mousePos = GetMousePosition();
    bool hoveredResume = CheckCollisionPointRec(mousePos, btnResume);
    bool hoveredMenu = CheckCollisionPointRec(mousePos, btnMenu);
    
    DrawThemeButton(btnResume, "RETOMAR (ESC)", 14, hoveredResume, COLOR_NEON_GREEN);
    DrawThemeButton(btnMenu, "SAIR PARA O MENU", 14, hoveredMenu, COLOR_NEON_RED);
}

void UpdateGame(void) {
    // Fullscreen toggle (F11 on Windows/Linux; Ctrl+F on macOS — F11 lowers volume on Mac)
#ifdef __APPLE__
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_F)) ToggleFullscreen();
#else
    if (IsKeyPressed(KEY_F11)) ToggleFullscreen();
#endif

    if (gamePaused) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            gamePaused = false;
            return;
        }
        
        Rectangle card = { SCREEN_WIDTH / 2.0f - 200, SCREEN_HEIGHT / 2.0f - 120, 400, 240 };
        Rectangle btnResume = { SCREEN_WIDTH / 2.0f - 150, card.y + 100, 300, 45 };
        Rectangle btnMenu = { SCREEN_WIDTH / 2.0f - 150, card.y + 160, 300, 45 };
        
        Vector2 mousePos = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mousePos, btnResume)) {
                gamePaused = false;
            }
            else if (CheckCollisionPointRec(mousePos, btnMenu)) {
                gamePaused = false;
                currentScreen = SCREEN_TITLE;
            }
        }
        return; // Congela o jogo
    }

    switch (currentScreen) {
        case SCREEN_TITLE:
            UpdateTitleScreen();
            break;
        case SCREEN_QUIZ:
            UpdateQuizScreen();
            break;
        case SCREEN_GAMEPLAY:
            UpdateGameplayScreen();
            break;
        case SCREEN_BOSS:
            UpdateBossScreen();
            break;
        case SCREEN_RANKING:
            UpdateRankingScreen();
            break;
        default:
            break;
    }
}

void DrawGame(void) {
    // NOTE: BeginTextureMode is called by main.c before this; do NOT call BeginDrawing here
    switch (currentScreen) {
        case SCREEN_TITLE:
            DrawTitleScreen();
            break;
        case SCREEN_QUIZ:
            DrawQuizScreen();
            break;
        case SCREEN_GAMEPLAY:
            DrawGameplayScreen();
            break;
        case SCREEN_BOSS:
            DrawBossScreen();
            break;
        case SCREEN_RANKING:
            DrawRankingScreen();
            break;
        default:
            ClearBackground(DARKGRAY);
            DrawText("Tela desconhecida!", 20, 20, 20, RED);
            break;
    }

    if (gamePaused) {
        DrawPauseOverlay();
    }
}

void UnloadGame(void) {
    UnloadTitleScreen();
    UnloadQuizScreen();
    UnloadGameplayScreen();
    UnloadBossScreen();
    
    // Unload global fonts
    UnloadFont(fontMain);
    UnloadFont(fontBold);
}
