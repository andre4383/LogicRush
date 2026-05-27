// ==================================================================================
//              FASE 3 — MEETING EQUAL — LOGIC RUSH
//  (Suporta Modo Historia com diálogos via dialogue.c)
//   A IA "Equal" lança cubos com proposições lógicas.
//   Clique na opção correta com o mouse antes do timer acabar.
//
//   Layout: Equal (ESQUERDA) lança cubos → direita → Jogador (DIREITA)
//   Estágio 1 (3 opções): Avaliar V/F — proposições simples
//   Estágio 2 (3 opções): Avaliar V/F — proposições compostas
//   Estágio 3 (4 opções): Selecionar equivalência lógica correta
// ==================================================================================

#include "raylib.h"
#include "../core/game.h"
#include "../core/ranking.h"
#include "../core/theme.h"
#include "../dialogue/dialogue.h"
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

// ==================================================================================
//                              CONSTANTES
// ==================================================================================

#define MAX_CUBES        6
#define BOSS_MAX_HP      100
#define PLAYER_MAX_HP    3

#define CUBE_W           230
#define CUBE_H           92

#define CUBE_TIME_S1     12.0f
#define CUBE_TIME_S2     10.0f
#define CUBE_TIME_S3     8.5f

#define SPAWN_RATE_S1    4.5f
#define SPAWN_RATE_S2    3.2f
#define SPAWN_RATE_S3    2.2f

#define BOSS_HP_STAGE2   66
#define BOSS_HP_STAGE3   33

#define BOSS_DAMAGE      14
#define PLAYER_DAMAGE    1

// Zona dos cartões de resposta (fundo da tela)
#define CARD_ZONE_Y      (SCREEN_HEIGHT - 155)
#define CARD_H           65

// ── Layout do confronto ──────────────────────────────────────────────────────────
// Equal fica à ESQUERDA, Jogador fica à DIREITA
// Cubos se movem da esquerda para a direita
#define EQUAL_CENTER_X   200.0f       // Equal fica mais à esquerda
#define EQUAL_CENTER_Y   310.0f       // centro vertical da Equal
#define PLAYER_CENTER_X  1020.0f      // centro horizontal do jogador
#define PLAYER_CENTER_Y  410.0f       // centro vertical do jogador

// Cubo: nasce da mão direita da Equal e morre quando alcança o escudo do jogador
#define CUBE_SPAWN_X     420.0f       // X de nascimento do cubo (mão da Equal)
#define CUBE_DEATH_X     860.0f       // X de morte do cubo (escudo do jogador)
#define CUBE_Y_MIN       170.0f       // banda vertical mínima para cubos
#define CUBE_Y_MAX       500.0f       // banda vertical máxima para cubos

// ==================================================================================
//                         BANCO DE PROPOSIÇÕES
// ==================================================================================

typedef struct {
    char expression[56];
    char prefix[52];
    char options[4][52];
    int  numOptions;
    int  stage;
    Color accentColor;
} Proposition;

static const Proposition PROPOSITIONS[] = {

// ── ESTÁGIO 1 ─────────────────────────────────────────────────────────────────────
    { "P ^ Q",    "Avalie com P=Verdadeiro, Q=Falso:",
      {"Falso (F)", "Verdadeiro (V)", "Tautologia", ""},          3, 1, {34,  211, 238, 255} },
    { "P v Q",    "Avalie com P=Verdadeiro, Q=Falso:",
      {"Verdadeiro (V)", "Falso (F)", "Contradicao", ""},         3, 1, {34,  197, 94,  255} },
    { "~P",       "Avalie com P=Verdadeiro:",
      {"Falso (F)", "Verdadeiro (V)", "Contingencia", ""},        3, 1, {168, 85,  247, 255} },
    { "~Q",       "Avalie com Q=Falso:",
      {"Verdadeiro (V)", "Falso (F)", "Indeterminado", ""},       3, 1, {234, 179, 8,   255} },
    { "P -> Q",   "Avalie com P=Verdadeiro, Q=Falso:",
      {"Falso (F)", "Verdadeiro (V)", "Contingencia", ""},        3, 1, {34,  197, 94,  255} },
    { "Q -> P",   "Avalie com P=Verdadeiro, Q=Falso:",
      {"Verdadeiro (V)", "Falso (F)", "Tautologia", ""},          3, 1, {34,  211, 238, 255} },
    { "P <-> Q",  "Avalie com P=Verdadeiro, Q=Falso:",
      {"Falso (F)", "Verdadeiro (V)", "Indeterminado", ""},       3, 1, {168, 85,  247, 255} },
    { "P ^ ~Q",   "Avalie com P=Verdadeiro, Q=Falso:",
      {"Verdadeiro (V)", "Falso (F)", "Contradicao", ""},         3, 1, {234, 179, 8,   255} },
    { "~P v Q",   "Avalie com P=Verdadeiro, Q=Falso:",
      {"Falso (F)", "Verdadeiro (V)", "Contingencia", ""},        3, 1, {34,  197, 94,  255} },
    { "~P ^ ~Q",  "Avalie com P=Verdadeiro, Q=Falso:",
      {"Falso (F)", "Verdadeiro (V)", "Tautologia", ""},          3, 1, {34,  211, 238, 255} },
    { "~P v ~Q",  "Avalie com P=Verdadeiro, Q=Falso:",
      {"Verdadeiro (V)", "Falso (F)", "Contradicao", ""},         3, 1, {168, 85,  247, 255} },
    { "P -> ~Q",  "Avalie com P=Verdadeiro, Q=Falso:",
      {"Verdadeiro (V)", "Falso (F)", "Indeterminado", ""},       3, 1, {234, 179, 8,   255} },
    { "~Q -> P",  "Avalie com P=Verdadeiro, Q=Falso:",
      {"Verdadeiro (V)", "Falso (F)", "Contingencia", ""},        3, 1, {34,  211, 238, 255} },
    { "P v ~P",   "Esta proposicao e classificada como:",
      {"Tautologia", "Contradicao", "Contingencia", ""},          3, 1, {34,  197, 94,  255} },
    { "P ^ ~P",   "Esta proposicao e classificada como:",
      {"Contradicao", "Tautologia", "Contingencia", ""},          3, 1, {168, 85,  247, 255} },
    { "~~P",      "Avalie com P=Verdadeiro:",
      {"Verdadeiro (V)", "Falso (F)", "Indeterminado", ""},       3, 1, {234, 179, 8,   255} },

// ── ESTÁGIO 2 ─────────────────────────────────────────────────────────────────────
    { "~(P ^ Q)",      "Avalie com P=Verdadeiro, Q=Falso:",
      {"Verdadeiro (V)", "Falso (F)", "Impossivel determinar", ""}, 3, 2, {217, 70,  239, 255} },
    { "~(P v Q)",      "Avalie com P=Verdadeiro, Q=Falso:",
      {"Falso (F)", "Verdadeiro (V)", "Impossivel determinar", ""}, 3, 2, {34,  211, 238, 255} },
    { "P <-> ~Q",      "Avalie com P=Verdadeiro, Q=Falso:",
      {"Verdadeiro (V)", "Falso (F)", "Depende do contexto", ""},   3, 2, {34,  197, 94,  255} },
    { "(P v Q) ^ ~Q",  "Avalie com P=Verdadeiro, Q=Falso:",
      {"Verdadeiro (V)", "Falso (F)", "Tautologia", ""},            3, 2, {168, 85,  247, 255} },
    { "P -> (P v Q)",  "Avalie com P=Verdadeiro, Q=Falso:",
      {"Verdadeiro (V)", "Falso (F)", "Depende de Q", ""},          3, 2, {234, 179, 8,   255} },
    { "(P->Q) -> P",   "Avalie com P=Verdadeiro, Q=Falso:",
      {"Verdadeiro (V)", "Falso (F)", "Impossivel", ""},            3, 2, {217, 70,  239, 255} },
    { "~P <-> ~Q",     "Avalie com P=Verdadeiro, Q=Falso:",
      {"Falso (F)", "Verdadeiro (V)", "Tautologia", ""},            3, 2, {34,  211, 238, 255} },
    { "~(~P v ~Q)",    "Avalie com P=Verdadeiro, Q=Falso:",
      {"Falso (F)", "Verdadeiro (V)", "Contradicao", ""},           3, 2, {34,  197, 94,  255} },
    { "(P ^ Q) -> Q",  "Avalie com P=Verdadeiro, Q=Falso:",
      {"Verdadeiro (V)", "Falso (F)", "Depende de P", ""},          3, 2, {168, 85,  247, 255} },
    { "P -> (Q -> P)", "Avalie com P=Verdadeiro, Q=Falso:",
      {"Verdadeiro (V)", "Falso (F)", "Depende de Q", ""},          3, 2, {234, 179, 8,   255} },
    { "(P v ~P) -> Q", "Avalie com P=Verdadeiro, Q=Falso:",
      {"Falso (F)", "Verdadeiro (V)", "Tautologia", ""},            3, 2, {217, 70,  239, 255} },
    { "~P v (P ^ Q)",  "Avalie com P=Verdadeiro, Q=Falso:",
      {"Falso (F)", "Verdadeiro (V)", "Depende de Q", ""},          3, 2, {34,  211, 238, 255} },
    { "(P ^ ~P) -> Q", "Esta proposicao e sempre:",
      {"Verdadeiro (V)", "Falso (F)", "Contingente", ""},           3, 2, {34,  197, 94,  255} },
    { "~(P <-> Q)",    "Avalie com P=Verdadeiro, Q=Falso:",
      {"Verdadeiro (V)", "Falso (F)", "Impossivel", ""},            3, 2, {168, 85,  247, 255} },
    { "P ^ (Q v ~Q)",  "Avalie com P=Verdadeiro:",
      {"Verdadeiro (V)", "Falso (F)", "Depende de Q", ""},          3, 2, {234, 179, 8,   255} },

// ── ESTÁGIO 3 (4 opções) ──────────────────────────────────────────────────────────
    { "~(P ^ Q)",
      "Selecione a equivalencia correta (De Morgan):",
      {"~P v ~Q", "~P ^ ~Q", "P v Q", "P -> ~Q"},
      4, 3, {239, 68,  68,  255} },
    { "~(P v Q)",
      "Selecione a equivalencia correta (De Morgan):",
      {"~P ^ ~Q", "~P v ~Q", "P ^ Q", "~P -> ~Q"},
      4, 3, {217, 70,  239, 255} },
    { "P -> Q",
      "Selecione a equivalencia correta:",
      {"~P v Q", "P ^ ~Q", "~P ^ Q", "P v ~Q"},
      4, 3, {34,  211, 238, 255} },
    { "P -> Q",
      "Selecione a contrapositiva equivalente:",
      {"~Q -> ~P", "~P -> ~Q", "Q -> P", "~P -> Q"},
      4, 3, {34,  197, 94,  255} },
    { "~~P",
      "Simplifica para (dupla negacao):",
      {"P", "~P", "P v ~P", "P ^ ~P"},
      4, 3, {168, 85,  247, 255} },
    { "P ^ (P -> Q)",
      "Simplifica para:",
      {"P ^ Q", "P v Q", "~P v Q", "Q -> P"},
      4, 3, {234, 179, 8,   255} },
    { "P v (P ^ Q)",
      "Simplifica para (Absorcao):",
      {"P", "P ^ Q", "P v Q", "Q"},
      4, 3, {239, 68,  68,  255} },
    { "P ^ (P v Q)",
      "Simplifica para (Absorcao):",
      {"P", "P v Q", "P ^ Q", "Q"},
      4, 3, {217, 70,  239, 255} },
    { "(P->Q) ^ (Q->P)",
      "Esta expressao equivale a:",
      {"P <-> Q", "P -> Q", "P ^ Q", "P v Q"},
      4, 3, {34,  211, 238, 255} },
    { "P <-> Q",
      "Selecione a forma expandida equivalente:",
      {"(P^Q)v(~P^~Q)", "P->Q", "P^Q", "~P^~Q"},
      4, 3, {34,  197, 94,  255} },
    { "P ^ Q",
      "Selecione a forma equivalente (comutatividade):",
      {"Q ^ P", "P v Q", "~P ^ ~Q", "P -> Q"},
      3, 3, {168, 85,  247, 255} },
    { "P v Q",
      "Selecione a forma equivalente (comutatividade):",
      {"Q v P", "P ^ Q", "~P v ~Q", "P -> Q"},
      3, 3, {234, 179, 8,   255} },
    { "P v ~P",
      "Esta proposicao e classificada como:",
      {"Tautologia", "Contradicao", "Contingencia", "Indefinida"},
      4, 3, {239, 68,  68,  255} },
    { "P ^ ~P",
      "Esta proposicao e classificada como:",
      {"Contradicao", "Tautologia", "Contingencia", "Tautologia condicional"},
      4, 3, {217, 70,  239, 255} },
    { "~(P <-> Q)",
      "Selecione a equivalencia correta:",
      {"(P^~Q) v (~P^Q)", "P ^ Q", "~P ^ ~Q", "P v Q"},
      4, 3, {34,  211, 238, 255} },
};

#define TOTAL_PROPOSITIONS ((int)(sizeof(PROPOSITIONS) / sizeof(PROPOSITIONS[0])))

// ==================================================================================
//                         ESTRUTURAS DE DADOS
// ==================================================================================

typedef struct {
    const Proposition *prop;
    char options[4][52];
    int  correctOption;
    int  numOptions;
    int  isActive;
    Vector2 position;       // position.x avança da esquerda para a direita
    float timeLeft;
    float totalTime;
    float fallSpeed;        // agora é velocidade HORIZONTAL (px/s)
    int   answered;
    int   playerChoice;
    int   wasCorrect;
    float resultTimer;
} LogicCube;

typedef enum {
    BPHASE_INTRO,
    BPHASE_FIGHTING,
    BPHASE_BOSS_HIT,
    BPHASE_PLAYER_HIT,
    BPHASE_VICTORY,
    BPHASE_DEFEAT,
    BPHASE_STORY_VICTORY_POPUP  // Modo História: popup de sistema após vitória
} BossPhase;

typedef struct {
    int   bossHP;
    int   playerHP;
    int   score;
    int   currentStage;
    float spawnTimer;
    float spawnCooldown;
    BossPhase phase;
    float phaseTimer;
    float shakeTimer;
    float attackTimer;
    float shieldTimer;
    Vector2 shieldPos;
} BossState;

// ==================================================================================
//                              ESTADO GLOBAL
// ==================================================================================

// Flags de controle do Modo História (resetadas em InitBossScreen)
static bool storyVictoryStarted  = false;
static bool storyDefeatHandled   = false;
static bool rankingTriggeredBoss = false;
static bool rankingTrigDBoss     = false;

static BossState boss;
static LogicCube cubes[MAX_CUBES];
static int       selectedCube;

static int       feedbackCube;
static float     feedbackTimer;

static Rectangle answerCards[4];
static int       hoveredCard;
static int       numActiveCards;

static int recentProps[5];

static unsigned int rng;

static unsigned int RandNext(void) {
    rng = rng * 1664525u + 1013904223u;
    return rng;
}

static int RandRange(int lo, int hi) {
    return lo + (int)((RandNext() >> 8) % (unsigned int)(hi - lo));
}

static void SCopy(char *dst, const char *src, int maxLen) {
    int i = 0;
    while (i < maxLen - 1 && src[i]) { dst[i] = src[i]; i++; }
    dst[i] = '\0';
}

// ==================================================================================
//              RECURSOS VISUAIS — SPRITE SHEETS
//
//  equal_spritesheet.png  : Grade 3 colunas × 4 linhas = 12 frames
//  ┌──────┬──────┬──────┐
//  │  0   │  1   │  2   │  Linha 0: HP cheio  — Idle / Cast / Attack
//  │  3   │  4   │  5   │  Linha 1: Reações   — Follow-through / Hurt / Aggro
//  │  6   │  7   │  8   │  Linha 2: Danificada — Idle / Cast / Attack
//  │  9   │  10  │  11  │  Linha 3: Fúria/Morte — Cast fury / Atk fury / Death
//  └──────┴──────┴──────┘
//
//  player_spritesheet.png : Grade 2 colunas × 2 linhas = 4 frames
//  ┌──────┬──────┐
//  │  0   │  1   │  Linha 0: Idle / Alert
//  │  2   │  3   │  Linha 1: Block / Hit
//  └──────┴──────┘
// ==================================================================================

// ── Equal sprite sheet ────────────────────────────────────────────────────────────
#define EQUAL_SHEET_COLS    3
#define EQUAL_SHEET_ROWS    4
#define EQUAL_DRAW_H        320.0f      // altura de destino na tela (px)

// Índices de frame da Equal
#define EQ_IDLE_N           0   // Idle normal (HP cheio)
#define EQ_CAST_N           1   // Cast normal
#define EQ_ATK_N            2   // Attack normal
#define EQ_FOLLOW           3   // Follow-through
#define EQ_HURT             4   // Tomando dano
#define EQ_AGGRO            5   // Aggro / phase shift (HP ≤ 30%)
#define EQ_IDLE_D           6   // Idle danificada (HP 30-66%)
#define EQ_CAST_D           7   // Cast danificada
#define EQ_ATK_D            8   // Attack danificada
#define EQ_CAST_F           9   // Cast fúria (HP ≤ 30%)
#define EQ_ATK_F            10  // Attack fúria
#define EQ_DEATH            11  // Derrotada

// ── Player sprite sheet ───────────────────────────────────────────────────────────
#define PLAYER_SHEET_COLS   2
#define PLAYER_SHEET_ROWS   2
#define PLAYER_DRAW_H       290.0f      // altura de destino na tela (px)

// Índices de frame do jogador
#define PL_IDLE             0   // Idle / Ready
#define PL_ALERT            1   // Alert / Stance
#define PL_BLOCK            2   // Active Shield Block (acerto)
#define PL_HIT              3   // Impact / Stagger (erro/timeout)

// ── Duração do flash de dano da Equal ────────────────────────────────────────────
#define EQUAL_DAMAGE_FLASH  0.55f

// ── Texturas globais ──────────────────────────────────────────────────────────────
static Texture2D texBackground;
static bool      texBackgroundLoaded = false;

static Texture2D texEqualSheet;
static bool      texEqualLoaded      = false;

static Texture2D texPlayerSheet;
static bool      texPlayerLoaded     = false;

// ── Estado de animação da Equal ───────────────────────────────────────────────────
static int   equalCurrentFrame  = EQ_IDLE_N;
static float equalDamageTimer   = 0.0f;

// ── Estado de animação do Jogador ─────────────────────────────────────────────────
static int   playerCurrentFrame = PL_IDLE;
static float playerBlockTimer   = 0.0f;    // duração do frame de bloqueio
static float playerHitTimer     = 0.0f;    // duração do frame de impacto
static float playerIdleToggle   = 0.0f;    // timer para alternar idle 0↔1

// ==================================================================================
//                    SELEÇÃO DE FRAME — EQUAL  (inline, sem função separada)
// ==================================================================================
// Chamado dentro de DrawEqualSprite

// ==================================================================================
//                    SELEÇÃO DE FRAME — JOGADOR (inline, sem função separada)
// ==================================================================================
// Chamado dentro de DrawPlayerSprite

// ==================================================================================
//                    DRAW — EQUAL (sprite sheet, lado esquerdo)
// ==================================================================================

static void DrawEqualSprite(float t) {
    // ── Seleciona frame ───────────────────────────────────────────────────────────
    if (boss.phase == BPHASE_VICTORY) {
        equalCurrentFrame = EQ_DEATH;
    } else if (equalDamageTimer > 0.0f) {
        equalCurrentFrame = EQ_HURT;
    } else if (boss.attackTimer > 0.0f) {
        float prog = boss.attackTimer / 0.55f;   // 1.0 → 0.0
        if (boss.currentStage == 1) {
            if      (prog > 0.5f)  equalCurrentFrame = EQ_CAST_N;
            else                   equalCurrentFrame = EQ_ATK_N;
        } else if (boss.currentStage == 2) {
            // Keep within damaged row (6,7,8) to avoid frame jumps
            if      (prog > 0.5f)  equalCurrentFrame = EQ_CAST_D;
            else                   equalCurrentFrame = EQ_ATK_D;
        } else {
            if (prog > 0.5f) equalCurrentFrame = EQ_CAST_F;
            else             equalCurrentFrame = EQ_ATK_F;
        }
    } else {
        if      (boss.currentStage == 1) equalCurrentFrame = EQ_IDLE_N;
        else if (boss.currentStage == 2) equalCurrentFrame = EQ_IDLE_D;
        else                             equalCurrentFrame = EQ_AGGRO;
    }

    if (!texEqualLoaded) {
        // Fallback visual: retângulo colorido enquanto textura não carrega
        float fbW = 200.0f, fbH = EQUAL_DRAW_H;
        float fbX = EQUAL_CENTER_X - fbW * 0.5f;
        float fbY = EQUAL_CENTER_Y - fbH * 0.5f + sinf(t * 2.2f) * 4.5f;
        Color fbCol = (boss.currentStage == 3) ? COLOR_NEON_RED :
                      (boss.currentStage == 2) ? COLOR_NEON_GOLD : COLOR_NEON_CYAN;
        DrawRectangle((int)fbX, (int)fbY, (int)fbW, (int)fbH, ColorAlpha(fbCol, 0.12f));
        DrawRectangleLinesEx((Rectangle){fbX,fbY,fbW,fbH}, 2.0f, ColorAlpha(fbCol,0.6f));
        DrawText("EQUAL", (int)(EQUAL_CENTER_X - MeasureText("EQUAL",16)*0.5f),
                 (int)(EQUAL_CENTER_Y - 8), 16, ColorAlpha(fbCol, 0.9f));
        DrawText("[sprite nao carregada]",
                 (int)(EQUAL_CENTER_X - MeasureText("[sprite nao carregada]",10)*0.5f),
                 (int)(EQUAL_CENTER_Y + 12), 10, ColorAlpha(COLOR_TEXT_MUTED, 0.6f));
        return;
    }

    // ── Geometria do frame ────────────────────────────────────────────────────────
    float frameW = roundf((float)texEqualSheet.width  / (float)EQUAL_SHEET_COLS);
    float frameH = roundf((float)texEqualSheet.height / (float)EQUAL_SHEET_ROWS);
    int   col    = equalCurrentFrame % EQUAL_SHEET_COLS;
    int   row    = equalCurrentFrame / EQUAL_SHEET_COLS;

    float inset = 1.0f;
    Rectangle srcRec = {
        col * frameW + inset,
        row * frameH + inset,
        frameW - inset * 2.0f,
        frameH - inset * 2.0f
    };

    float dstH  = EQUAL_DRAW_H;
    float dstW  = (frameW > 0.0f) ? dstH * (frameW / frameH) : dstH;
    float drawX = EQUAL_CENTER_X - dstW * 0.5f;
    float drawY = EQUAL_CENTER_Y - dstH * 0.5f;

    // ── Efeitos de posição ────────────────────────────────────────────────────────
    float bob    = sinf(t * 2.2f) * 4.5f;
    float shakeX = 0.0f;
    if (boss.shakeTimer > 0.0f && boss.phase == BPHASE_BOSS_HIT)
        shakeX = sinf(t * 90.0f) * 8.0f * (boss.shakeTimer / 0.4f);

    Rectangle dstRec = { drawX + shakeX, drawY + bob, dstW, dstH };

    // ── Tint dinâmico ─────────────────────────────────────────────────────────────
    Color tint = WHITE;
    if (equalCurrentFrame == EQ_HURT && equalDamageTimer > 0.0f) {
        float fa = equalDamageTimer / EQUAL_DAMAGE_FLASH;
        unsigned char wb = (unsigned char)(200 + (int)(55.0f * fa));
        tint = (Color){ wb, wb, wb, 255 };
    } else if (equalCurrentFrame == EQ_AGGRO || equalCurrentFrame == EQ_CAST_F
               || equalCurrentFrame == EQ_ATK_F) {
        float p = 0.82f + sinf(t * 6.5f) * 0.15f;
        tint = (Color){ 255, (unsigned char)(170.0f * p), (unsigned char)(170.0f * p), 255 };
    } else if (equalCurrentFrame == EQ_ATK_N || equalCurrentFrame == EQ_ATK_D) {
        tint = (Color){ 255, 235, 90, 255 };
    } else if (equalCurrentFrame == EQ_DEATH) {
        // Escurece gradualmente na derrota
        tint = (Color){ 160, 160, 180, 255 };
    }

    DrawTexturePro(texEqualSheet, srcRec, dstRec, (Vector2){0,0}, 0.0f, tint);

    // ── Partículas extras ─────────────────────────────────────────────────────────
    // Raios na fúria/aggro
    if (equalCurrentFrame == EQ_AGGRO || equalCurrentFrame == EQ_CAST_F
        || equalCurrentFrame == EQ_ATK_F) {
        float cx = dstRec.x + dstW * 0.5f;
        float cy = dstRec.y + dstH * 0.38f;
        for (int i = 0; i < 4; i++) {
            float angle = t * 2.8f + i * 1.5708f;
            float r1    = dstH * 0.28f;
            float r2    = r1 + 22.0f + sinf(t * 22.0f + i) * 10.0f;
            float flick = 0.55f + sinf(t * 35.0f + i * 1.3f) * 0.40f;
            DrawLineEx(
                (Vector2){ cx + cosf(angle)*r1, cy + sinf(angle)*r1 },
                (Vector2){ cx + cosf(angle)*r2, cy + sinf(angle)*r2 },
                2.5f, ColorAlpha(COLOR_NEON_RED, flick));
            DrawCircleV(
                (Vector2){ cx + cosf(angle)*r2, cy + sinf(angle)*r2 },
                4.0f, ColorAlpha(COLOR_NEON_RED, flick * 0.7f));
        }
    }

    // Aura de carga no cast/ataque
    if (equalCurrentFrame == EQ_CAST_N || equalCurrentFrame == EQ_CAST_D
        || equalCurrentFrame == EQ_CAST_F) {
        float prog  = (boss.attackTimer > 0.0f) ? (boss.attackTimer / 0.55f) : 0.0f;
        float raise = sinf((1.0f - prog) * 3.14159f * 0.85f);
        Color glow  = (boss.currentStage == 3) ? COLOR_NEON_RED :
                      (boss.currentStage == 2) ? COLOR_NEON_GOLD : COLOR_NEON_CYAN;
        // A mão direita da Equal fica em ~70% da largura do sprite
        float handX = dstRec.x + dstW * 0.70f;
        float handY = dstRec.y + dstH * 0.42f;
        DrawCircle((int)handX, (int)handY, (int)(28 * raise),
                   ColorAlpha(glow, raise * 0.25f));
        DrawCircleLines((int)handX, (int)handY, (int)(28 * raise),
                        ColorAlpha(WHITE, raise * 0.50f));
    }

    // Nome abaixo da sprite
    Color nameCol = (boss.currentStage == 3 || equalCurrentFrame == EQ_HURT
                     || equalCurrentFrame == EQ_DEATH)
                    ? COLOR_NEON_RED : COLOR_NEON_CYAN;
    const char *nm = "E Q U A L";
    DrawText(nm,
             (int)(EQUAL_CENTER_X - (float)MeasureText(nm, 13) * 0.5f),
             (int)(dstRec.y + dstH + 4.0f),
             13, ColorAlpha(nameCol, 0.88f));
}

// ==================================================================================
//                    DRAW — JOGADOR (sprite sheet, lado direito)
// ==================================================================================

static void DrawPlayerSprite(float t) {
    // ── Seleciona frame ───────────────────────────────────────────────────────────
    if (boss.phase == BPHASE_PLAYER_HIT && playerHitTimer > 0.0f) {
        playerCurrentFrame = PL_HIT;
    } else if (boss.shieldTimer > 0.0f && playerBlockTimer > 0.0f) {
        playerCurrentFrame = PL_BLOCK;
    } else {
        // Alterna idle 0↔1 a cada 0.5s para animação de prontidão
        int idxIdle = (int)(t * 2.0f) % 2;
        playerCurrentFrame = (idxIdle == 0) ? PL_IDLE : PL_ALERT;
    }

    if (!texPlayerLoaded) {
        // Fallback visual: retângulo colorido enquanto textura não carrega
        float fbW = 160.0f, fbH = PLAYER_DRAW_H;
        float fbX = PLAYER_CENTER_X - fbW * 0.5f;
        float fbY = PLAYER_CENTER_Y - fbH * 0.5f + sinf(t * 2.2f) * 3.0f;
        Color fbCol = (boss.phase == BPHASE_PLAYER_HIT) ? COLOR_NEON_RED : COLOR_NEON_CYAN;
        DrawRectangle((int)fbX, (int)fbY, (int)fbW, (int)fbH, ColorAlpha(fbCol, 0.12f));
        DrawRectangleLinesEx((Rectangle){fbX,fbY,fbW,fbH}, 2.0f, ColorAlpha(fbCol,0.6f));
        DrawText("JOGADOR", (int)(PLAYER_CENTER_X - MeasureText("JOGADOR",16)*0.5f),
                 (int)(PLAYER_CENTER_Y - 8), 16, ColorAlpha(fbCol, 0.9f));
        DrawText("[sprite nao carregada]",
                 (int)(PLAYER_CENTER_X - MeasureText("[sprite nao carregada]",10)*0.5f),
                 (int)(PLAYER_CENTER_Y + 12), 10, ColorAlpha(COLOR_TEXT_MUTED, 0.6f));
        return;
    }

    // ── Geometria do frame ────────────────────────────────────────────────────────
    float frameW = floorf((float)texPlayerSheet.width  / (float)PLAYER_SHEET_COLS);
    float frameH = floorf((float)texPlayerSheet.height / (float)PLAYER_SHEET_ROWS);
    int   col    = playerCurrentFrame % PLAYER_SHEET_COLS;
    int   row    = playerCurrentFrame / PLAYER_SHEET_COLS;

    float inset = 1.0f;
    Rectangle srcRec = {
        col * frameW + inset,
        row * frameH + inset,
        frameW - inset * 2.0f,
        frameH - inset * 2.0f
    };

    float dstH  = PLAYER_DRAW_H;
    float dstW  = (frameW > 0.0f) ? dstH * (frameW / frameH) : dstH;
    float drawX = PLAYER_CENTER_X - dstW * 0.5f;
    float drawY = PLAYER_CENTER_Y - dstH * 0.5f;

    // ── Efeitos de posição ────────────────────────────────────────────────────────
    float bob    = sinf(t * 2.2f) * 3.0f;
    float shakeX = 0.0f;
    if (boss.shakeTimer > 0.0f && boss.phase == BPHASE_PLAYER_HIT)
        shakeX = sinf(t * 85.0f) * 7.0f * (boss.shakeTimer / 0.35f);

    Rectangle dstRec = { drawX + shakeX, drawY + bob, dstW, dstH };

    // ── Tint dinâmico ─────────────────────────────────────────────────────────────
    Color tint = WHITE;
    if (playerCurrentFrame == PL_HIT) {
        // Flash vermelho no impacto
        float fa = playerHitTimer / 0.35f;
        tint = (Color){ 255, (unsigned char)(200 * (1.0f - fa * 0.4f)),
                             (unsigned char)(200 * (1.0f - fa * 0.4f)), 255 };
    } else if (playerCurrentFrame == PL_BLOCK) {
        // Tint ciano brilhante no bloqueio
        tint = (Color){ 180, 255, 255, 255 };
    }

    DrawTexturePro(texPlayerSheet, srcRec, dstRec, (Vector2){0,0}, 0.0f, tint);

    // ── Aura de escudo no bloqueio ────────────────────────────────────────────────
    if (playerCurrentFrame == PL_BLOCK && playerBlockTimer > 0.0f) {
        float alpha = playerBlockTimer / 0.85f;
        float shieldX = dstRec.x + dstW * 0.20f;   // escudo fica à esquerda do sprite
        float shieldY = dstRec.y + dstH * 0.45f;
        DrawCircle((int)shieldX, (int)shieldY, 55,
                   ColorAlpha(COLOR_NEON_CYAN, alpha * 0.15f));
        DrawCircleLines((int)shieldX, (int)shieldY, 55,
                        ColorAlpha(COLOR_NEON_CYAN, alpha * 0.80f));
        // Hexágono interno (linhas)
        for (int i = 0; i < 6; i++) {
            float a1 = i * 60.0f * DEG2RAD + t * 1.5f;
            float a2 = (i + 1) * 60.0f * DEG2RAD + t * 1.5f;
            DrawLineEx(
                (Vector2){ shieldX + cosf(a1)*45, shieldY + sinf(a1)*45 },
                (Vector2){ shieldX + cosf(a2)*45, shieldY + sinf(a2)*45 },
                2.0f, ColorAlpha(COLOR_NEON_CYAN, alpha * 0.55f));
        }
    }

    // ── Partículas de impacto ─────────────────────────────────────────────────────
    if (playerCurrentFrame == PL_HIT && playerHitTimer > 0.0f) {
        float alpha  = playerHitTimer / 0.35f;
        float sparX  = dstRec.x + dstW * 0.45f;
        float sparY  = dstRec.y + dstH * 0.38f;
        for (int i = 0; i < 5; i++) {
            float a = t * 60.0f + i * 72.0f * DEG2RAD;
            float r = 30.0f + sinf(t * 25.0f + i) * 12.0f;
            DrawCircle((int)(sparX + cosf(a)*r), (int)(sparY + sinf(a)*r),
                       3, ColorAlpha(COLOR_NEON_CYAN, alpha));
        }
    }
}

// ==================================================================================
//                         LAYOUT DOS CARTÕES
// ==================================================================================

static void UpdateCardLayout(int numCards) {
    float cardW, gap;
    if      (numCards == 2) { cardW = 380.0f; gap = 28.0f; }
    else if (numCards == 3) { cardW = 290.0f; gap = 20.0f; }
    else                    { cardW = 264.0f; gap = 13.0f; }

    float totalW = numCards * cardW + (numCards - 1) * gap;
    float startX = (SCREEN_WIDTH - totalW) / 2.0f;

    for (int i = 0; i < numCards; i++) {
        answerCards[i] = (Rectangle){
            startX + i * (cardW + gap),
            (float)CARD_ZONE_Y,
            cardW,
            (float)CARD_H
        };
    }
    numActiveCards = numCards;
}

// ==================================================================================
//                              INIT / UNLOAD
// ==================================================================================

void InitBossScreen(void) {
    // Reseta flags de história
    storyVictoryStarted  = false;
    storyDefeatHandled   = false;
    rankingTriggeredBoss = false;
    rankingTrigDBoss     = false;

    boss.bossHP        = BOSS_MAX_HP;
    boss.playerHP      = (globalLives > 0 && globalLives <= PLAYER_MAX_HP)
                         ? globalLives : PLAYER_MAX_HP;
    boss.currentStage  = 1;
    boss.spawnTimer    = 1.5f;
    boss.spawnCooldown = SPAWN_RATE_S1;
    boss.phase         = BPHASE_INTRO;
    boss.phaseTimer    = 3.0f;
    boss.shakeTimer    = 0.0f;
    boss.attackTimer   = 0.0f;
    boss.shieldTimer   = 0.0f;
    boss.shieldPos     = (Vector2){ PLAYER_CENTER_X, PLAYER_CENTER_Y };

    rng = (unsigned int)(GetTime() * 100000.0) ^ 0xDEADBEEFu;

    for (int i = 0; i < MAX_CUBES; i++) {
        cubes[i].isActive    = 0;
        cubes[i].answered    = 0;
        cubes[i].prop        = NULL;
        cubes[i].playerChoice = -1;
    }
    for (int i = 0; i < 5; i++) recentProps[i] = -1;

    selectedCube   = -1;
    feedbackCube   = -1;
    feedbackTimer  = 0.0f;
    hoveredCard    = -1;
    numActiveCards = 0;
    UpdateCardLayout(2);

    // ── Animação reset ───────────────────────────────────────────────────────────
    equalCurrentFrame  = EQ_IDLE_N;
    equalDamageTimer   = 0.0f;
    playerCurrentFrame = PL_IDLE;
    playerBlockTimer   = 0.0f;
    playerHitTimer     = 0.0f;
    playerIdleToggle   = 0.0f;

    // ── Carregamento de texturas ─────────────────────────────────────────────────
    if (!texBackgroundLoaded) {
        texBackground       = LoadTexture("assets/images/cenario_boss.png");
        texBackgroundLoaded = (texBackground.id > 0);
    }
    if (!texEqualLoaded) {
        texEqualSheet = LoadTexture("assets/images/equal_spritesheet.png");
        if (texEqualSheet.id == 0)   // tenta extensão alternativa (.jpg)
            texEqualSheet = LoadTexture("assets/images/equal_spritesheet.jpg");
        texEqualLoaded = (texEqualSheet.id > 0);
    }
    if (!texPlayerLoaded) {
        texPlayerSheet = LoadTexture("assets/images/player_spritesheet.png");
        if (texPlayerSheet.id == 0)  // tenta extensão alternativa (.jpg)
            texPlayerSheet = LoadTexture("assets/images/player_spritesheet.jpg");
        texPlayerLoaded = (texPlayerSheet.id > 0);
    }
}

void UnloadBossScreen(void) {
    if (texBackgroundLoaded) { UnloadTexture(texBackground);  texBackgroundLoaded = false; }
    if (texEqualLoaded)      { UnloadTexture(texEqualSheet);  texEqualLoaded      = false; }
    if (texPlayerLoaded)     { UnloadTexture(texPlayerSheet); texPlayerLoaded     = false; }
}

// ==================================================================================
//                              SPAWN DE CUBO
// ==================================================================================

static void SpawnCube(void) {
    int slot = -1;
    for (int i = 0; i < MAX_CUBES; i++) {
        if (!cubes[i].isActive) { slot = i; break; }
    }
    if (slot < 0) return;

    // Seleciona proposição
    // No estágio 3, intercala 50% V/F (estágio 2) para aliviar a dificuldade
    int targetStage = boss.currentStage;
    // Alívio de dificuldade: 50% chance de estágio 2 no estágio 3, apenas nos primeiros 2 loops
    if (boss.currentStage == 3 && competitiveLoop < 2 && (RandRange(0, 2) == 0)) {
        targetStage = 2;
    }
    int idx = -1;
    int attempts = 0;
    while (attempts < 200) {
        int candidate = RandRange(0, TOTAL_PROPOSITIONS);
        if (PROPOSITIONS[candidate].stage != targetStage) { attempts++; continue; }
        int repeated = 0;
        for (int r = 0; r < 5; r++) {
            if (recentProps[r] == candidate) { repeated = 1; break; }
        }
        if (!repeated) { idx = candidate; break; }
        attempts++;
    }
    if (idx < 0) {
        for (int i = 0; i < TOTAL_PROPOSITIONS; i++) {
            if (PROPOSITIONS[i].stage == targetStage) { idx = i; break; }
        }
    }

    for (int i = 4; i > 0; i--) recentProps[i] = recentProps[i - 1];
    recentProps[0] = idx;

    const Proposition *p = &PROPOSITIONS[idx];
    int n = p->numOptions;

    // Embaralha opções
    int perm[4] = {0, 1, 2, 3};
    for (int i = n - 1; i > 0; i--) {
        int j = RandRange(0, i + 1);
        int tmp = perm[i]; perm[i] = perm[j]; perm[j] = tmp;
    }

    int newCorrect = 0;
    for (int i = 0; i < n; i++) {
        SCopy(cubes[slot].options[i], p->options[perm[i]], 52);
        if (perm[i] == 0) newCorrect = i;
    }

    // ── Anti-sobreposição em Y (cubos se movem no eixo X, variam em Y) ───────────
    float yMin  = CUBE_Y_MIN;
    float yMax  = CUBE_Y_MAX - CUBE_H;
    float bestY = yMin + (float)(RandRange(0, (int)(yMax - yMin)));
    float bestDist = 0.0f;

    for (int attempt = 0; attempt < 8; attempt++) {
        float cand = yMin + (float)(RandRange(0, (int)(yMax - yMin)));
        float minDist = 9999.0f;
        for (int ci = 0; ci < MAX_CUBES; ci++) {
            if (!cubes[ci].isActive || ci == slot) continue;
            float dy = fabsf(cand - cubes[ci].position.y);
            if (dy < minDist) minDist = dy;
        }
        if (minDist > bestDist) { bestDist = minDist; bestY = cand; }
    }

    // ── Velocidade horizontal: percurso de CUBE_SPAWN_X até CUBE_DEATH_X ─────────
    // A cada loop o tempo diminui 0.8s (blocos mais rápidos)
    float travelDist = CUBE_DEATH_X - CUBE_SPAWN_X;
    float loopSpeedup = (float)competitiveLoop * 0.8f;
    float timeWindow = (boss.currentStage == 1) ? fmaxf(4.0f, CUBE_TIME_S1 - loopSpeedup) :
                       (boss.currentStage == 2) ? fmaxf(3.5f, CUBE_TIME_S2 - loopSpeedup) :
                                                  fmaxf(3.0f, CUBE_TIME_S3 - loopSpeedup);
    float spd = (travelDist / timeWindow) * 1.08f;

    cubes[slot].prop          = p;
    cubes[slot].correctOption = newCorrect;
    cubes[slot].numOptions    = n;
    cubes[slot].isActive      = 1;
    cubes[slot].answered      = 0;
    cubes[slot].playerChoice  = -1;
    cubes[slot].wasCorrect    = 0;
    cubes[slot].position      = (Vector2){ CUBE_SPAWN_X, bestY };
    cubes[slot].fallSpeed     = spd;   // velocidade horizontal
    cubes[slot].totalTime     = timeWindow;
    cubes[slot].timeLeft      = timeWindow;
    cubes[slot].resultTimer   = 0.0f;

    boss.attackTimer = 0.55f;
}

// ==================================================================================
//                              UPDATE
// ==================================================================================

static void ApplyAnswer(int cubeIdx, int chosenOption) {
    int correct = (chosenOption == cubes[cubeIdx].correctOption);

    cubes[cubeIdx].answered    = 1;
    cubes[cubeIdx].playerChoice = chosenOption;
    cubes[cubeIdx].wasCorrect  = correct;
    cubes[cubeIdx].resultTimer = 0.9f;

    feedbackCube  = cubeIdx;
    feedbackTimer = 0.9f;

    if (correct) {
        boss.bossHP -= BOSS_DAMAGE;
        globalScore += 100 * boss.currentStage * scoreMultiplier;
        boss.shieldTimer  = 0.85f;
        boss.shieldPos    = (Vector2){ PLAYER_CENTER_X - 80.0f, PLAYER_CENTER_Y };
        playerBlockTimer  = 0.85f;
        if (boss.bossHP <= 0) {
            boss.bossHP     = 0;
            boss.phase      = BPHASE_VICTORY;
            boss.shakeTimer = 1.0f;
        } else {
            boss.phase      = BPHASE_BOSS_HIT;
            boss.phaseTimer = 0.4f;
            boss.shakeTimer = 0.4f;
        }
    } else {
        boss.playerHP -= PLAYER_DAMAGE;
        globalLives = boss.playerHP;  // sync
        playerHitTimer = 0.45f;
        if (boss.playerHP <= 0) {
            boss.playerHP = 0;
            globalLives   = 0;
            boss.phase    = BPHASE_DEFEAT;
        } else {
            boss.phase      = BPHASE_PLAYER_HIT;
            boss.phaseTimer = 0.35f;
            boss.shakeTimer = 0.35f;
        }
    }
}

static void ApplyTimeout(int cubeIdx) {
    cubes[cubeIdx].answered    = 1;
    cubes[cubeIdx].playerChoice = -1;
    cubes[cubeIdx].wasCorrect  = 0;
    cubes[cubeIdx].resultTimer = 0.9f;

    feedbackCube  = cubeIdx;
    feedbackTimer = 0.9f;

    boss.playerHP -= PLAYER_DAMAGE;
    globalLives = boss.playerHP;  // sync
    playerHitTimer = 0.45f;
    if (boss.playerHP <= 0) {
        boss.playerHP = 0;
        globalLives   = 0;
        boss.phase    = BPHASE_DEFEAT;
    } else {
        boss.phase      = BPHASE_PLAYER_HIT;
        boss.phaseTimer = 0.35f;
        boss.shakeTimer = 0.35f;
    }
}

void UpdateBossScreen(void) {
    float dt = GetFrameTime();

    if (phaseBannerTimer > 0.0f) {
        UpdatePhaseBanner(dt);
        return;
    }

    // ── Modo História: diálogos como overlay (pausa o boss) ──────────────────
    if (Dialogue_IsActive()) {
        Dialogue_Update();
        return;
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (boss.phase != BPHASE_VICTORY && boss.phase != BPHASE_DEFEAT
            && boss.phase != BPHASE_STORY_VICTORY_POPUP)
            gamePaused = true;
        else
            currentScreen = SCREEN_TITLE;
        return;
    }

    if (boss.phase == BPHASE_FIGHTING || boss.phase == BPHASE_BOSS_HIT
        || boss.phase == BPHASE_PLAYER_HIT) {
        globalTimer += dt;
    }

    // ── BPHASE_INTRO ──────────────────────────────────────────────────────────
    if (boss.phase == BPHASE_INTRO) {
        boss.phaseTimer -= dt;
        if (boss.phaseTimer <= 0.0f) {
            if (currentGameMode == MODE_STORY) {
                // Modo História: mostra diálogos de abertura antes de começar
                Dialogue_StartSeq(DSEQ_BOSS_OPEN, SCREEN_BOSS);
                boss.phase = BPHASE_FIGHTING; // já prepara o estado; diálogos pausam
            } else {
                boss.phase = BPHASE_FIGHTING;
            }
        }
        return;
    }

    // ── BPHASE_VICTORY ────────────────────────────────────────────────────────
    if (boss.phase == BPHASE_VICTORY) {
        if (currentGameMode == MODE_STORY) {
            if (!storyVictoryStarted) {
                storyVictoryStarted = true;
                // Mostra diálogo final de Equal
                Dialogue_StartSeq(DSEQ_BOSS_DEFEAT, SCREEN_BOSS);
            } else if (!Dialogue_IsActive()) {
                // Diálogos terminaram → popup de sistema
                boss.phase = BPHASE_STORY_VICTORY_POPUP;
            }
        } else {
            // Modo Competitivo: loop para próximo ciclo com dificuldade maior
            if (!rankingTriggeredBoss) {
                rankingTriggeredBoss = true;
                competitiveLoop++;
                scoreMultiplier = competitiveLoop + 1;
                static char loopTitle[32];
                static char loopSubtitle[52];
                snprintf(loopTitle,    sizeof(loopTitle),    "CICLO %d", competitiveLoop + 1);
                snprintf(loopSubtitle, sizeof(loopSubtitle), "MULTIPLICADOR x%d  DIFICULDADE+", scoreMultiplier);
                InitQuizScreen();
                StartPhaseBanner(loopTitle, loopSubtitle);
                currentScreen = SCREEN_QUIZ;
            }
        }
        return;
    }

    // ── BPHASE_STORY_VICTORY_POPUP ────────────────────────────────────────────
    if (boss.phase == BPHASE_STORY_VICTORY_POPUP) {
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)
            || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            currentScreen = SCREEN_TITLE;
        }
        return;
    }

    // ── BPHASE_DEFEAT ─────────────────────────────────────────────────────────
    if (boss.phase == BPHASE_DEFEAT) {
        if (currentGameMode == MODE_STORY) {
            // Modo História: sem ranking, volta ao menu
            if (!storyDefeatHandled) {
                storyDefeatHandled = true;
                currentScreen = SCREEN_TITLE;
            }
        } else {
            if (!rankingTrigDBoss) {
                rankingTrigDBoss = true;
                Ranking_ScreenEnter(globalScore, globalTimer, 3);
                currentScreen = SCREEN_RANKING;
            }
            if (IsKeyPressed(KEY_ENTER)) {
                rankingTrigDBoss = false;
                currentScreen = SCREEN_TITLE;
            }
        }
        return;
    }

    if (boss.phase == BPHASE_BOSS_HIT || boss.phase == BPHASE_PLAYER_HIT) {
        boss.phaseTimer -= dt;
        boss.shakeTimer -= dt;
        if (boss.phaseTimer <= 0.0f) boss.phase = BPHASE_FIGHTING;
    }

    // Estágio — thresholds sobem a cada loop: estágios difíceis aparecem mais cedo
    int effStage2 = BOSS_HP_STAGE2 + competitiveLoop * 12;
    if (effStage2 > 90) effStage2 = 90;
    int effStage3 = BOSS_HP_STAGE3 + competitiveLoop * 17;
    if (effStage3 > effStage2 - 8) effStage3 = effStage2 - 8;
    if      (boss.bossHP > effStage2) boss.currentStage = 1;
    else if (boss.bossHP > effStage3) boss.currentStage = 2;
    else                               boss.currentStage = 3;

    boss.spawnCooldown = (boss.currentStage == 1) ? SPAWN_RATE_S1 :
                          (boss.currentStage == 2) ? SPAWN_RATE_S2 : SPAWN_RATE_S3;

    // Timers de feedback e animação
    if (feedbackTimer    > 0.0f) { feedbackTimer -= dt; if (feedbackTimer <= 0.0f) feedbackCube = -1; }
    if (equalDamageTimer > 0.0f) equalDamageTimer -= dt;
    if (boss.phase == BPHASE_BOSS_HIT && boss.phaseTimer >= 0.38f)
        equalDamageTimer = EQUAL_DAMAGE_FLASH;
    if (boss.attackTimer  > 0.0f) boss.attackTimer  -= dt;
    if (boss.shieldTimer  > 0.0f) boss.shieldTimer  -= dt;
    if (playerBlockTimer  > 0.0f) playerBlockTimer  -= dt;
    if (playerHitTimer    > 0.0f) playerHitTimer    -= dt;

    // ── Modo História: diálogos de threshold de HP ───────────────────────────
    if (currentGameMode == MODE_STORY && boss.phase == BPHASE_FIGHTING) {
        if (!storyBossPhase2Shown && boss.bossHP <= BOSS_HP_STAGE2) {
            storyBossPhase2Shown = true;
            Dialogue_StartSeq(DSEQ_BOSS_MID, SCREEN_BOSS);
        }
        if (!storyBossPhase3Shown && boss.bossHP <= BOSS_HP_STAGE3) {
            storyBossPhase3Shown = true;
            Dialogue_StartSeq(DSEQ_BOSS_LOW, SCREEN_BOSS);
        }
    }

    // Spawn
    if (boss.phase == BPHASE_FIGHTING) {
        boss.spawnTimer -= dt;
        if (boss.spawnTimer <= 0.0f) {
            boss.spawnTimer = boss.spawnCooldown;
            SpawnCube();
        }
    }

    // ── Seleciona cubo mais urgente: o que está mais à DIREITA (mais perto do jogador)
    int highestSlot = -1;
    float highestX  = -1.0f;
    for (int i = 0; i < MAX_CUBES; i++) {
        if (cubes[i].isActive && !cubes[i].answered) {
            if (cubes[i].position.x > highestX) {
                highestX    = cubes[i].position.x;
                highestSlot = i;
            }
        }
    }

    if (highestSlot != selectedCube) {
        selectedCube = highestSlot;
        if (selectedCube >= 0)
            UpdateCardLayout(cubes[selectedCube].numOptions);
        else
            numActiveCards = 0;
    }

    // Input
    Vector2 mouse = GetMousePosition();
    hoveredCard = -1;
    int acceptInput = (boss.phase == BPHASE_FIGHTING);

    if (acceptInput && selectedCube >= 0 && numActiveCards > 0) {
        for (int i = 0; i < numActiveCards; i++) {
            if (CheckCollisionPointRec(mouse, answerCards[i])) {
                hoveredCard = i;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    ApplyAnswer(selectedCube, i);
                    break;
                }
            }
        }
    }

    // ── Atualiza cubos (movimento HORIZONTAL: X cresce) ───────────────────────────
    for (int i = 0; i < MAX_CUBES; i++) {
        if (!cubes[i].isActive) continue;

        if (cubes[i].answered) {
            cubes[i].resultTimer -= dt;
            if (cubes[i].resultTimer <= 0.0f) cubes[i].isActive = 0;
            continue;
        }

        cubes[i].position.x += cubes[i].fallSpeed * dt;   // move para a direita
        cubes[i].timeLeft   -= dt;

        // Cubo alcança o escudo do jogador sem resposta
        if (cubes[i].timeLeft <= 0.0f || cubes[i].position.x > CUBE_DEATH_X) {
            ApplyTimeout(i);
        }
    }
}

// ==================================================================================
//                         DRAW — RAIO ELÉTRICO (helper)
// ==================================================================================

static void DrawLightningBolt(Vector2 from, Vector2 to, float t, float seed,
                               float thickness, Color col) {
    float dx = to.x - from.x, dy = to.y - from.y;
    float len = sqrtf(dx*dx + dy*dy);
    if (len < 1.0f) return;
    float px = -dy / len, py = dx / len;
    int segs = 9;
    Vector2 prev = from;
    for (int i = 1; i <= segs; i++) {
        float prog = (float)i / segs;
        float bx   = from.x + dx * prog;
        float by   = from.y + dy * prog;
        float amp  = len * 0.13f * sinf(prog * 3.14159f);
        float zz   = sinf(t * 42.0f + i * 1.9f + seed * 3.7f) * amp;
        Vector2 curr = { bx + px * zz, by + py * zz };
        DrawLineEx(prev, curr, thickness, col);
        prev = curr;
    }
    DrawLineEx(prev, to, thickness, col);
}

// ==================================================================================
//                         DRAW — CUBO LÓGICO
// ==================================================================================

static void DrawLogicCube(LogicCube *cube, int isSelected, float t) {
    float x = cube->position.x - CUBE_W / 2.0f;
    float y = cube->position.y;
    Rectangle rect = { x, y, CUBE_W, CUBE_H };
    Color accent = cube->prop->accentColor;
    Color bg, border;

    if (cube->answered) {
        Color rc = cube->wasCorrect ? COLOR_NEON_GREEN : COLOR_NEON_RED;
        bg     = ColorAlpha(rc, 0.22f);
        border = rc;
    } else {
        float pulse = sinf(t * 7.0f) * 0.04f;
        bg     = ColorAlpha(accent, isSelected ? (0.16f + pulse) : 0.07f);
        border = isSelected ? ColorAlpha(accent, 0.85f) : ColorAlpha(accent, 0.38f);
    }

    DrawRectangleRounded(rect, 0.14f, 6, bg);
    DrawRectangleRoundedLines(rect, 0.14f, 6, border);

    if (isSelected && !cube->answered) {
        DrawRectangleRoundedLines(
            (Rectangle){x-3,y-3,CUBE_W+6,CUBE_H+6}, 0.16f, 6,
            ColorAlpha(accent, 0.22f + sinf(t*8)*0.09f));
    }

    int exprSize = 24;
    int exprW = MeasureText(cube->prop->expression, exprSize);
    int exprY = (cube->prop->stage == 3) ? (int)(y + 10) : (int)(y + CUBE_H/2 - 14);
    DrawText(cube->prop->expression,
             (int)(x + CUBE_W/2 - exprW/2), exprY,
             exprSize, cube->answered ? WHITE : accent);

    if (cube->answered) {
        const char *res = cube->wasCorrect ? "CORRETO!" :
                          (cube->playerChoice < 0) ? "TEMPO ESGOTADO!" : "ERRADO!";
        int rw = MeasureText(res, 15);
        DrawText(res, (int)(x+CUBE_W/2-rw/2), (int)(y+50), 15,
                 cube->wasCorrect ? COLOR_NEON_GREEN : COLOR_NEON_RED);
        return;
    }

    if (cube->prop->stage == 3) {
        int pfxSize = 11;
        int pfxW    = MeasureText(cube->prop->prefix, pfxSize);
        if (pfxW > CUBE_W - 10) pfxSize = 10;
        DrawText(cube->prop->prefix, (int)(x+CUBE_W/2-pfxW/2), (int)(y+46),
                 pfxSize, ColorAlpha(COLOR_NEON_PURPLE, 0.80f));
    }

    // Barra de tempo (horizontal, no topo do cubo)
    float ratio = cube->timeLeft / cube->totalTime;
    if (ratio < 0) ratio = 0;
    Color barCol = ratio > 0.5f ? COLOR_NEON_GREEN :
                   ratio > 0.25f ? COLOR_NEON_GOLD : COLOR_NEON_RED;
    DrawRectangle((int)(x+10), (int)(y+CUBE_H-11), CUBE_W-20, 6,
                  ColorAlpha(COLOR_PANEL_BORDER, 0.8f));
    DrawRectangle((int)(x+10), (int)(y+CUBE_H-11), (int)((CUBE_W-20)*ratio), 6,
                  barCol);

    // Efeito de nascimento
    if (ratio > 0.86f) {
        float sa  = (ratio - 0.86f) / 0.14f;
        float flk = 0.5f + sinf((float)GetTime() * 55.0f) * 0.5f;
        DrawRectangleRoundedLines(
            (Rectangle){ x - 5, y - 5, CUBE_W + 10, CUBE_H + 10 },
            0.14f, 6, ColorAlpha(COLOR_NEON_CYAN, sa * flk));
        DrawRectangleRoundedLines(
            (Rectangle){ x - 2, y - 2, CUBE_W + 4, CUBE_H + 4 },
            0.14f, 6, ColorAlpha(WHITE, sa * flk * 0.45f));

        float cx2 = (float)GetTime();
        Vector2 corners[4] = {
            { x,         y },
            { x + CUBE_W, y },
            { x,         y + CUBE_H },
            { x + CUBE_W, y + CUBE_H }
        };
        for (int c = 0; c < 4; c++) {
            float ang = cx2 * 90.0f + c * 90.0f;
            float spkLen = 8.0f + sa * 10.0f;
            DrawLineEx(corners[c],
                       (Vector2){ corners[c].x + cosf(ang*DEG2RAD)*spkLen,
                                  corners[c].y + sinf(ang*DEG2RAD)*spkLen },
                       2.0f, ColorAlpha(COLOR_NEON_CYAN, sa * 0.9f));
        }
        DrawLightningBolt(
            (Vector2){x, y + CUBE_H/2.0f},
            (Vector2){x + CUBE_W, y + CUBE_H/2.0f},
            (float)GetTime(), 3.0f, 1.5f, ColorAlpha(COLOR_NEON_CYAN, sa*0.60f));
    }
}

// ==================================================================================
//                         DRAW — CARTÕES DE RESPOSTA
// ==================================================================================

static void DrawAnswerCards(float t) {
    int displayCube = -1;
    int isFeedback  = 0;

    if (feedbackTimer > 0.0f && feedbackCube >= 0) {
        displayCube = feedbackCube;
        isFeedback  = 1;
    } else if (selectedCube >= 0) {
        displayCube = selectedCube;
    }

    if (displayCube < 0 || numActiveCards == 0) return;

    LogicCube *cube = &cubes[displayCube];

    Rectangle zone = { 35.0f, (float)(CARD_ZONE_Y - 26),
                        SCREEN_WIDTH - 70.0f, (float)(CARD_H + 48) };
    DrawRectangleRounded(zone, 0.10f, 6, ColorAlpha(COLOR_PANEL_BG, 0.80f));
    DrawRectangleRoundedLines(zone, 0.10f, 6, ColorAlpha(COLOR_GRID_LINE, 0.20f));

    const char *zoneLabel = isFeedback ? "Resposta:" : "Clique na opcao correta:";
    int zlw = MeasureText(zoneLabel, 13);
    DrawText(zoneLabel, SCREEN_WIDTH/2 - zlw/2, CARD_ZONE_Y - 20, 13,
             ColorAlpha(COLOR_TEXT_MUTED, 0.75f));

    for (int i = 0; i < numActiveCards; i++) {
        Color accent = cube->prop->accentColor;
        Color bg, border, textCol;

        if (isFeedback) {
            int isCorrect = (i == cube->correctOption);
            int isChosen  = (i == cube->playerChoice);
            if (isCorrect) {
                bg = ColorAlpha(COLOR_NEON_GREEN, 0.22f);
                border  = COLOR_NEON_GREEN;
                textCol = COLOR_NEON_GREEN;
            } else if (isChosen && !isCorrect) {
                bg = ColorAlpha(COLOR_NEON_RED, 0.22f);
                border  = COLOR_NEON_RED;
                textCol = COLOR_NEON_RED;
            } else {
                bg = ColorAlpha(COLOR_PANEL_BG, 0.5f);
                border  = ColorAlpha(COLOR_PANEL_BORDER, 0.3f);
                textCol = ColorAlpha(COLOR_TEXT_MUTED, 0.4f);
            }
        } else {
            int hov = (i == hoveredCard);
            float pulse = sinf(t * 6.0f) * 0.04f;
            bg      = hov ? ColorAlpha(accent, 0.18f + pulse) : ColorAlpha(COLOR_PANEL_BG, 0.90f);
            border  = hov ? ColorAlpha(accent, 0.90f)  : ColorAlpha(COLOR_PANEL_BORDER, 0.55f);
            textCol = hov ? WHITE : COLOR_TEXT_MUTED;
            if (hov) {
                DrawRectangleRoundedLines(
                    (Rectangle){answerCards[i].x-3, answerCards[i].y-3,
                                answerCards[i].width+6, answerCards[i].height+6},
                    0.18f, 6, ColorAlpha(accent, 0.22f + sinf(t*8)*0.09f));
            }
        }

        DrawRectangleRounded(answerCards[i], 0.15f, 6, bg);
        DrawRectangleRoundedLines(answerCards[i], 0.15f, 6, border);

        const char *letter = (i==0)?"A":(i==1)?"B":(i==2)?"C":"D";
        DrawText(letter, (int)(answerCards[i].x + 9), (int)(answerCards[i].y + 7),
                 14, ColorAlpha(accent, isFeedback ? 0.5f :
                                (i == hoveredCard ? 0.9f : 0.45f)));

        int fontSize = 16;
        int textW    = MeasureText(cube->options[i], fontSize);
        int maxW     = (int)(answerCards[i].width - 32);
        while (textW > maxW && fontSize > 11) {
            fontSize--;
            textW = MeasureText(cube->options[i], fontSize);
        }
        DrawText(cube->options[i],
                 (int)(answerCards[i].x + answerCards[i].width/2 - textW/2),
                 (int)(answerCards[i].y + answerCards[i].height/2 - fontSize/2),
                 fontSize, textCol);

        if (isFeedback) {
            if (i == cube->correctOption)
                DrawText("V", (int)(answerCards[i].x + answerCards[i].width - 22),
                         (int)(answerCards[i].y + 8), 16, COLOR_NEON_GREEN);
            else if (i == cube->playerChoice)
                DrawText("X", (int)(answerCards[i].x + answerCards[i].width - 22),
                         (int)(answerCards[i].y + 8), 16, COLOR_NEON_RED);
        }
    }
}

// ==================================================================================
//                         DRAW — ESCUDO (acerto do jogador)
// ==================================================================================

static void DrawPlayerShield(float t) {
    if (boss.shieldTimer <= 0.0f) return;

    float alpha = boss.shieldTimer / 0.85f;
    float scale = 1.0f + (1.0f - alpha) * 0.35f;
    float sx    = boss.shieldPos.x;
    float sy    = boss.shieldPos.y;
    Color sc    = COLOR_NEON_CYAN;
    float r     = 68.0f * scale;

    DrawCircle((int)sx, (int)sy, (int)(r+18), ColorAlpha(sc, alpha*0.06f));
    DrawCircle((int)sx, (int)sy, (int)(r+10), ColorAlpha(sc, alpha*0.12f));
    DrawCircle((int)sx, (int)sy, (int)r,      ColorAlpha(sc, alpha*0.22f));
    DrawCircleLines((int)sx, (int)sy, (int)r, ColorAlpha(sc, alpha));
    DrawCircleLines((int)sx, (int)sy, (int)(r-5), ColorAlpha(sc, alpha*0.45f));

    for (int i = 0; i < 6; i++) {
        float ang  = i * 60.0f * DEG2RAD + t * 1.2f;
        float ang2 = (i+1) * 60.0f * DEG2RAD + t * 1.2f;
        DrawLineEx(
            (Vector2){ sx + cosf(ang) *(r-8), sy + sinf(ang) *(r-8) },
            (Vector2){ sx + cosf(ang2)*(r-8), sy + sinf(ang2)*(r-8) },
            2.0f, ColorAlpha(sc, alpha*0.55f));
    }

    DrawLineEx((Vector2){sx-20,sy+2},  (Vector2){sx-6, sy+18}, 5.0f, ColorAlpha(COLOR_NEON_GREEN, alpha));
    DrawLineEx((Vector2){sx-6, sy+18}, (Vector2){sx+22,sy-16}, 5.0f, ColorAlpha(COLOR_NEON_GREEN, alpha));

    const char *txt = "BLOQUEADO!";
    DrawText(txt, (int)(sx - MeasureText(txt,20)/2), (int)(sy-r-26), 20,
             ColorAlpha(COLOR_NEON_GREEN, alpha));
}

// ==================================================================================
//                         DRAW — HUD
// ==================================================================================

// HP da Equal — barra vertical, lado esquerdo da tela
static void DrawBossHPBar(void) {
    int barH = 240, barW = 24;
    int bx   = 18;
    int by   = (SCREEN_HEIGHT - barH) / 2;

    DrawRectangle(bx, by, barW, barH, ColorAlpha(COLOR_PANEL_BG, 0.75f));
    DrawRectangleLinesEx((Rectangle){(float)bx,(float)by,(float)barW,(float)barH},
                         1.0f, ColorAlpha(COLOR_NEON_RED, 0.35f));

    float ratio  = (float)boss.bossHP / BOSS_MAX_HP;
    Color barCol = ratio > 0.66f ? COLOR_NEON_GREEN :
                   ratio > 0.33f ? COLOR_NEON_GOLD  : COLOR_NEON_RED;
    int fillH = (int)(barH * ratio);
    DrawRectangle(bx, by + barH - fillH, barW, fillH, ColorAlpha(barCol, 0.85f));

    for (int i = 0; i < 7; i++) {
        float midY = by + (float)barH/7*i + (float)barH/14;
        int   filled = (midY > (by + barH - fillH));
        DrawTriangle(
            (Vector2){(float)bx+2, midY-8},
            (Vector2){(float)bx+2, midY+8},
            (Vector2){(float)bx+barW-2, midY},
            filled ? ColorAlpha(barCol,0.9f) : ColorAlpha(COLOR_TEXT_MUTED,0.18f));
    }
    DrawText("EQUAL", bx-2, by-22, 13, COLOR_TEXT_MUTED);
    DrawText(TextFormat("%d%%", boss.bossHP), bx, by+barH+5, 12, ColorAlpha(barCol,0.85f));
}

// ==================================================================================
//                    DRAW — PAINEL DE VARIÁVEIS (legenda fixa, topo-centro)
// ==================================================================================

static void DrawParamLegend(float t) {
    Rectangle panel = { (float)(SCREEN_WIDTH/2 - 96), 52.0f, 192.0f, 78.0f };

    DrawRectangleRounded(panel, 0.16f, 6, ColorAlpha(COLOR_PANEL_BG, 0.92f));
    DrawRectangleRoundedLines(panel, 0.16f, 6,
        ColorAlpha(COLOR_NEON_CYAN, 0.26f + sinf(t * 3.0f) * 0.10f));
    DrawLineEx(
        (Vector2){ panel.x + 8,              panel.y + 1.0f },
        (Vector2){ panel.x + panel.width - 8, panel.y + 1.0f },
        1.5f, ColorAlpha(COLOR_NEON_CYAN, 0.55f));

    DrawText("VARIAVEIS:", (int)(panel.x + 10), (int)(panel.y + 7), 11,
             ColorAlpha(COLOR_TEXT_MUTED, 0.68f));

    float ry1 = panel.y + 26.0f;
    DrawText("P", (int)(panel.x + 10), (int)ry1, 20, COLOR_NEON_CYAN);
    DrawText("=", (int)(panel.x + 28), (int)(ry1 + 2), 14,
             ColorAlpha(COLOR_TEXT_MUTED, 0.60f));
    DrawRectangleRounded((Rectangle){ panel.x+44, ry1, 136.0f, 21.0f }, 0.30f, 4,
                          ColorAlpha(COLOR_NEON_GREEN, 0.14f));
    DrawRectangleRoundedLines((Rectangle){ panel.x+44, ry1, 136.0f, 21.0f }, 0.30f, 4,
                               ColorAlpha(COLOR_NEON_GREEN, 0.45f));
    DrawText("Verdadeiro", (int)(panel.x+50), (int)(ry1+5), 12, COLOR_NEON_GREEN);
    DrawText("(V)", (int)(panel.x+156), (int)(ry1+5), 12, ColorAlpha(COLOR_NEON_GREEN,0.72f));

    float ry2 = panel.y + 52.0f;
    DrawText("Q", (int)(panel.x + 10), (int)ry2, 20, COLOR_NEON_CYAN);
    DrawText("=", (int)(panel.x + 28), (int)(ry2 + 2), 14,
             ColorAlpha(COLOR_TEXT_MUTED, 0.60f));
    DrawRectangleRounded((Rectangle){ panel.x+44, ry2, 136.0f, 21.0f }, 0.30f, 4,
                          ColorAlpha(COLOR_NEON_RED, 0.14f));
    DrawRectangleRoundedLines((Rectangle){ panel.x+44, ry2, 136.0f, 21.0f }, 0.30f, 4,
                               ColorAlpha(COLOR_NEON_RED, 0.45f));
    DrawText("Falso", (int)(panel.x+50), (int)(ry2+5), 12, COLOR_NEON_RED);
    DrawText("(F)", (int)(panel.x+156), (int)(ry2+5), 12, ColorAlpha(COLOR_NEON_RED,0.72f));
}

// ==================================================================================
//                              DRAW PRINCIPAL
// ==================================================================================

void DrawBossScreen(void) {
    float t = (float)GetTime();

    // ── 1. FUNDO ──────────────────────────────────────────────────────────────────
    if (texBackgroundLoaded) {
        DrawTexturePro(
            texBackground,
            (Rectangle){0,0,(float)texBackground.width,(float)texBackground.height},
            (Rectangle){0,0,(float)SCREEN_WIDTH,(float)SCREEN_HEIGHT},
            (Vector2){0,0}, 0.0f, WHITE);
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha(BLACK, 0.42f));
    } else {
        ClearBackground(COLOR_BG_DARK);
        DrawThemeGrid(SCREEN_WIDTH, SCREEN_HEIGHT, CELL_SIZE);
    }

    // ── 2. LINHA DIVISÓRIA centro da arena ───────────────────────────────────────
    DrawLineEx((Vector2){(float)(SCREEN_WIDTH/2), 80.0f},
               (Vector2){(float)(SCREEN_WIDTH/2), (float)(CARD_ZONE_Y - 30)},
               1.0f, ColorAlpha(COLOR_GRID_LINE, 0.15f));

    // ── 3. FLASHES DE ESTADO ─────────────────────────────────────────────────────
    if (boss.playerHP == 1)
        DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,
                      ColorAlpha(COLOR_NEON_RED, 0.05f + sinf(t*4.5f)*0.03f));
    if (boss.phase == BPHASE_BOSS_HIT)
        DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,
                      ColorAlpha(COLOR_NEON_GREEN, 0.07f));
    if (boss.phase == BPHASE_PLAYER_HIT)
        DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,
                      ColorAlpha(COLOR_NEON_RED, 0.15f));

    // ── 4. PERSONAGENS E CUBOS ────────────────────────────────────────────────────
    DrawEqualSprite(t);           // Equal — lado ESQUERDO
    DrawPlayerSprite(t);          // Jogador — lado DIREITO

    for (int i = 0; i < MAX_CUBES; i++) {
        if (cubes[i].isActive)
            DrawLogicCube(&cubes[i], (i == selectedCube), t);
    }

    DrawPlayerShield(t);          // Efeito de escudo (sobre cubos)

    // ── 5. PAINEL DE VARIÁVEIS (P/Q) — centro-topo ───────────────────────────────
    DrawParamLegend(t);

    // ── 6. CARTÕES DE RESPOSTA ────────────────────────────────────────────────────
    DrawAnswerCards(t);

    // ── 7. HUD ────────────────────────────────────────────────────────────────────
    const char *stgStr = (boss.currentStage == 1) ? "ESTAGIO 1: INICIANTE" :
                          (boss.currentStage == 2) ? "ESTAGIO 2: AVANCADO"  :
                                                     "ESTAGIO 3: CRITICO!";
    DrawUnifiedHUD("FASE 3: BOSS FIGHT", stgStr,
                   "Clique com o MOUSE nas respostas para rebater os ataques");

    {
        const char *hint = (selectedCube >= 0 && feedbackTimer <= 0.0f)
            ? "Mouse: Clicar na resposta  |  Responda antes do tempo!  |  ESC: Pausar"
            : "Aguarde o proximo cubo...  |  ESC: Pausar";
        DrawBottomHUD(hint);
    }

    DrawBossHPBar();      // HP da Equal — barra vertical esquerda

    DrawThemeVignette(SCREEN_WIDTH, SCREEN_HEIGHT);

    // ── 8. OVERLAYS DE ESTADO ────────────────────────────────────────────────────
    if (boss.phase == BPHASE_INTRO) {
        DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ColorAlpha(BLACK,0.62f));
        const char *it = "M E E T I N G   E Q U A L";
        DrawText(it, SCREEN_WIDTH/2-MeasureText(it,40)/2, SCREEN_HEIGHT/2-35, 40,
                 ColorAlpha(COLOR_NEON_CYAN, 0.6f + sinf(t*3.5f)*0.3f));
        const char *is = "Clique na equivalencia logica correta para repelir os cubos!";
        DrawText(is, SCREEN_WIDTH/2-MeasureText(is,16)/2, SCREEN_HEIGHT/2+22, 16, COLOR_TEXT_MUTED);
        const char *is2 = "O cursor do mouse e sua unica arma.";
        DrawText(is2, SCREEN_WIDTH/2-MeasureText(is2,15)/2, SCREEN_HEIGHT/2+48, 15,
                 ColorAlpha(COLOR_TEXT_MUTED,0.65f));
    }

    if (boss.phase == BPHASE_VICTORY) {
        if (currentGameMode == MODE_STORY) {
            // Modo História: fundo escurecido durante diálogos de derrota
            DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ColorAlpha(BLACK,0.55f));
        } else {
            DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ColorAlpha(BLACK,0.70f));
            const char *wt = "EQUAL DERROTADA!";
            DrawText(wt, SCREEN_WIDTH/2-MeasureText(wt,56)/2, SCREEN_HEIGHT/2-72, 56,
                     COLOR_NEON_GREEN);
            const char *ws = "Os sistemas foram restaurados com sucesso!";
            DrawText(ws, SCREEN_WIDTH/2-MeasureText(ws,20)/2, SCREEN_HEIGHT/2, 20, COLOR_TEXT_MUTED);
            const char *sc = TextFormat("PONTUACAO FINAL: %d", globalScore);
            DrawText(sc, SCREEN_WIDTH/2-MeasureText(sc,26)/2, SCREEN_HEIGHT/2+36, 26, COLOR_NEON_GOLD);
            if (sinf(t*3.0f) > 0.0f) {
                const char *et = "Pressione ENTER para voltar ao Menu";
                DrawText(et, SCREEN_WIDTH/2-MeasureText(et,18)/2, SCREEN_HEIGHT/2+80, 18, COLOR_TEXT_MUTED);
            }
        }
    }

    // ── Popup de sistema (Modo História, pós-vitória) ────────────────────────
    if (boss.phase == BPHASE_STORY_VICTORY_POPUP) {
        DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ColorAlpha(BLACK,0.80f));

        // Painel central estilo terminal
        float pw = 760, ph = 200;
        float px = SCREEN_WIDTH/2.0f - pw/2.0f, py = SCREEN_HEIGHT/2.0f - ph/2.0f;
        Rectangle panel = {px, py, pw, ph};
        DrawThemeGlassPanel(panel, 0.06f, COLOR_NEON_GREEN);

        // Borda ciano pulsante
        float pulse = 0.6f + sinf(t*4.0f)*0.3f;
        DrawRectangleLinesEx(panel, 2.0f, ColorAlpha(COLOR_NEON_GREEN, pulse));

        // Texto da mensagem do sistema
        const char *line1 = "PROCESSO  'EQUAL.EXE'  ENCERRADO COM SUCESSO.";
        const char *line2 = "PRIVILEGIOS DE ADMINISTRADOR RESTAURADOS.";
        int l1w = MeasureTextBold(line1, 20);
        int l2w = MeasureTextBold(line2, 20);
        DrawTextBold(line1, (int)(px+pw/2-l1w/2), (int)(py+50), 20, COLOR_NEON_GREEN);
        DrawTextBold(line2, (int)(px+pw/2-l2w/2), (int)(py+82), 20, COLOR_NEON_GREEN);

        // Dica piscante
        if (sinf(t*3.0f) > 0.0f) {
            const char *hint = "[ ENTER ] Finalizar";
            int hw = MeasureText(hint, 15);
            DrawText(hint, (int)(px+pw/2-hw/2), (int)(py+140), 15, COLOR_TEXT_MUTED);
        }
    }

    if (boss.phase == BPHASE_DEFEAT) {
        DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ColorAlpha(BLACK,0.78f));
        const char *lt = "SISTEMA COMPROMETIDO";
        DrawText(lt, SCREEN_WIDTH/2-MeasureText(lt,50)/2, SCREEN_HEIGHT/2-70, 50, COLOR_NEON_RED);
        const char *ls = "Equal tomou controle total dos sistemas!";
        DrawText(ls, SCREEN_WIDTH/2-MeasureText(ls,20)/2, SCREEN_HEIGHT/2+4, 20, COLOR_TEXT_MUTED);
        if (currentGameMode != MODE_STORY && sinf(t*3.0f) > 0.0f) {
            const char *rt = "Pressione ENTER para tentar novamente";
            DrawText(rt, SCREEN_WIDTH/2-MeasureText(rt,18)/2, SCREEN_HEIGHT/2+50, 18, COLOR_TEXT_MUTED);
        }
    }

    // ── Modo História: diálogo como overlay por cima de tudo ─────────────────
    if (Dialogue_IsActive()) {
        Dialogue_Draw();
    }

    DrawPhaseBanner();
}
