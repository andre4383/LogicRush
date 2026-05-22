// ==================================================================================
//              FASE 3 — MEETING EQUAL — LOGIC RUSH
//   A IA "Equal" lança cubos com proposições lógicas.
//   Clique na opção correta com o mouse antes do timer acabar.
//
//   Estágio 1 (2 opções): Avaliar V/F — proposições simples
//   Estágio 2 (3 opções): Avaliar V/F — proposições compostas
//   Estágio 3 (4 opções): Selecionar equivalência lógica correta
// ==================================================================================

#include "raylib.h"
#include "../core/game.h"
#include "../core/theme.h"
#include <stddef.h>
#include <string.h>
#include <math.h>

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

// Zona dos cartões de resposta
#define CARD_ZONE_Y      (SCREEN_HEIGHT - 155)
#define CARD_H           65

// ==================================================================================
//                         BANCO DE PROPOSIÇÕES
// ==================================================================================
// Regra: options[0] é SEMPRE a resposta correta no struct.
// SpawnCube embaralha as opções e rastreia a nova posição correta.
// Isso garante aleatoriedade na posição das respostas a cada partida.

typedef struct {
    char expression[56];   // Expressão exibida no cubo
    char prefix[52];       // Tipo de questão mostrado abaixo da expressão
    char options[4][52];   // [0] = correto, [1..3] = distratores
    int  numOptions;       // 2, 3 ou 4
    int  stage;
    Color accentColor;
} Proposition;

static const Proposition PROPOSITIONS[] = {

// ── ESTÁGIO 1: Avaliar — Proposições Simples (3 opções: V, F + distrator lógico) ─

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

// ── ESTÁGIO 2: Avaliar V/F — Proposições Compostas (3 opções) ─────────────────

    { "~(P ^ Q)",      "Avalie com P=Verdadeiro, Q=Falso:",
      {"Verdadeiro (V)", "Falso (F)", "Impossivel determinar", ""},
      3, 2, {217, 70,  239, 255} },

    { "~(P v Q)",      "Avalie com P=Verdadeiro, Q=Falso:",
      {"Falso (F)", "Verdadeiro (V)", "Impossivel determinar", ""},
      3, 2, {34,  211, 238, 255} },

    { "P <-> ~Q",      "Avalie com P=Verdadeiro, Q=Falso:",
      {"Verdadeiro (V)", "Falso (F)", "Depende do contexto", ""},
      3, 2, {34,  197, 94,  255} },

    { "(P v Q) ^ ~Q",  "Avalie com P=Verdadeiro, Q=Falso:",
      {"Verdadeiro (V)", "Falso (F)", "Tautologia", ""},
      3, 2, {168, 85,  247, 255} },

    { "P -> (P v Q)",  "Avalie com P=Verdadeiro, Q=Falso:",
      {"Verdadeiro (V)", "Falso (F)", "Depende de Q", ""},
      3, 2, {234, 179, 8,   255} },

    { "(P->Q) -> P",   "Avalie com P=Verdadeiro, Q=Falso:",
      {"Verdadeiro (V)", "Falso (F)", "Impossivel", ""},
      3, 2, {217, 70,  239, 255} },

    { "~P <-> ~Q",     "Avalie com P=Verdadeiro, Q=Falso:",
      {"Falso (F)", "Verdadeiro (V)", "Tautologia", ""},
      3, 2, {34,  211, 238, 255} },

    { "~(~P v ~Q)",    "Avalie com P=Verdadeiro, Q=Falso:",
      {"Falso (F)", "Verdadeiro (V)", "Contradicao", ""},
      3, 2, {34,  197, 94,  255} },

    { "(P ^ Q) -> Q",  "Avalie com P=Verdadeiro, Q=Falso:",
      {"Verdadeiro (V)", "Falso (F)", "Depende de P", ""},
      3, 2, {168, 85,  247, 255} },

    { "P -> (Q -> P)", "Avalie com P=Verdadeiro, Q=Falso:",
      {"Verdadeiro (V)", "Falso (F)", "Depende de Q", ""},
      3, 2, {234, 179, 8,   255} },

    { "(P v ~P) -> Q", "Avalie com P=Verdadeiro, Q=Falso:",
      {"Falso (F)", "Verdadeiro (V)", "Tautologia", ""},
      3, 2, {217, 70,  239, 255} },

    { "~P v (P ^ Q)",  "Avalie com P=Verdadeiro, Q=Falso:",
      {"Falso (F)", "Verdadeiro (V)", "Depende de Q", ""},
      3, 2, {34,  211, 238, 255} },

    { "(P ^ ~P) -> Q", "Esta proposicao e sempre:",
      {"Verdadeiro (V)", "Falso (F)", "Contingente", ""},
      3, 2, {34,  197, 94,  255} },

    { "~(P <-> Q)",    "Avalie com P=Verdadeiro, Q=Falso:",
      {"Verdadeiro (V)", "Falso (F)", "Impossivel", ""},
      3, 2, {168, 85,  247, 255} },

    { "P ^ (Q v ~Q)",  "Avalie com P=Verdadeiro:",
      {"Verdadeiro (V)", "Falso (F)", "Depende de Q", ""},
      3, 2, {234, 179, 8,   255} },

// ── ESTÁGIO 3: Equivalências Lógicas (4 opções) ───────────────────────────────

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

    { "P ^ (Q v R)",
      "Aplique a distributividade:",
      {"(P^Q) v (P^R)", "(PvQ) ^ (PvR)", "P v (Q^R)", "P^Q^R"},
      4, 3, {168, 85,  247, 255} },

    { "(P v Q) ^ (P v R)",
      "Aplique a distributividade:",
      {"P v (Q ^ R)", "(P^Q) v (P^R)", "P ^ (Q v R)", "P v Q v R"},
      4, 3, {234, 179, 8,   255} },

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

// Um cubo lançado pela boss com opções já embaralhadas
typedef struct {
    const Proposition *prop;    // Referência à proposição original
    char options[4][52];        // Cópia local com opções EMBARALHADAS
    int  correctOption;         // Índice correto APÓS o embaralhamento
    int  numOptions;
    int  isActive;
    Vector2 position;
    float timeLeft;
    float totalTime;
    float fallSpeed;
    int   answered;             // 1 = respondido (correto ou errado)
    int   playerChoice;         // Qual opção o jogador clicou (-1 = timeout)
    int   wasCorrect;           // Se acertou
    float resultTimer;          // Tempo de exibição do feedback
} LogicCube;

typedef enum {
    BPHASE_INTRO,
    BPHASE_FIGHTING,
    BPHASE_BOSS_HIT,
    BPHASE_PLAYER_HIT,
    BPHASE_VICTORY,
    BPHASE_DEFEAT
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
    float attackTimer;   // Arm throw animation (when cube is spawned)
    float shieldTimer;   // Shield block animation (when player is correct)
    Vector2 shieldPos;   // Screen position where shield appears
} BossState;

// ==================================================================================
//                              ESTADO GLOBAL
// ==================================================================================

static BossState boss;
static LogicCube cubes[MAX_CUBES];
static int       selectedCube;  // Cubo mais urgente (mais baixo), recebe input

// Feedback visual do último cubo respondido
static int       feedbackCube;
static float     feedbackTimer;

// Cartões de resposta
static Rectangle answerCards[4];
static int       hoveredCard;
static int       numActiveCards;

// Histórico para evitar repetições seguidas
static int recentProps[5];

// Gerador pseudo-aleatório (LCG)
static unsigned int rng;

static unsigned int RandNext(void) {
    rng = rng * 1664525u + 1013904223u;
    return rng;
}

static int RandRange(int lo, int hi) {   // [lo, hi)
    return lo + (int)((RandNext() >> 8) % (unsigned int)(hi - lo));
}

// Copia string de forma segura (sem depender de string.h externamente)
static void SCopy(char *dst, const char *src, int maxLen) {
    int i = 0;
    while (i < maxLen - 1 && src[i]) { dst[i] = src[i]; i++; }
    dst[i] = '\0';
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
    boss.bossHP        = BOSS_MAX_HP;
    boss.playerHP      = PLAYER_MAX_HP;
    boss.currentStage  = 1;
    boss.spawnTimer    = 1.5f;
    boss.spawnCooldown = SPAWN_RATE_S1;
    boss.phase         = BPHASE_INTRO;
    boss.phaseTimer    = 3.0f;
    boss.shakeTimer    = 0.0f;
    boss.attackTimer   = 0.0f;
    boss.shieldTimer   = 0.0f;
    boss.shieldPos     = (Vector2){ SCREEN_WIDTH / 2.0f, CARD_ZONE_Y - 80.0f };

    // Semente com tempo atual para variedade real a cada partida
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
}

void UnloadBossScreen(void) { /* sem recursos dinâmicos */ }

// ==================================================================================
//                              SPAWN DE CUBO
// ==================================================================================

static void SpawnCube(void) {
    // Encontra slot livre
    int slot = -1;
    for (int i = 0; i < MAX_CUBES; i++) {
        if (!cubes[i].isActive) { slot = i; break; }
    }
    if (slot < 0) return;

    // Escolhe proposição aleatória do estágio atual, evitando repetições recentes
    int idx = -1;
    int attempts = 0;
    while (attempts < 200) {
        int candidate = RandRange(0, TOTAL_PROPOSITIONS);
        if (PROPOSITIONS[candidate].stage != boss.currentStage) { attempts++; continue; }

        // Verifica se não é repetição recente
        int repeated = 0;
        for (int r = 0; r < 5; r++) {
            if (recentProps[r] == candidate) { repeated = 1; break; }
        }
        if (!repeated) { idx = candidate; break; }
        attempts++;
    }
    // Fallback se não encontrou candidato válido
    if (idx < 0) {
        for (int i = 0; i < TOTAL_PROPOSITIONS; i++) {
            if (PROPOSITIONS[i].stage == boss.currentStage) { idx = i; break; }
        }
    }

    // Registra no histórico de recentes (roda circular)
    for (int i = 4; i > 0; i--) recentProps[i] = recentProps[i - 1];
    recentProps[0] = idx;

    const Proposition *p = &PROPOSITIONS[idx];
    int n = p->numOptions;

    // ── Embaralha as opções (Fisher-Yates) ──────────────────────────────────────
    int perm[4] = {0, 1, 2, 3};
    for (int i = n - 1; i > 0; i--) {
        int j = RandRange(0, i + 1);
        int tmp = perm[i]; perm[i] = perm[j]; perm[j] = tmp;
    }

    // Copia opções na nova ordem e encontra posição correta
    int newCorrect = 0;
    for (int i = 0; i < n; i++) {
        SCopy(cubes[slot].options[i], p->options[perm[i]], 52);
        if (perm[i] == 0) newCorrect = i;  // options[0] é sempre o correto no struct
    }

    // ── Anti-sobreposição: escolhe X com maior distância de cubos ativos ─────────
    float xMin  = 50.0f;
    float xMax  = (float)SCREEN_WIDTH - CUBE_W - 50.0f;
    float bestX = xMin + (float)(RandRange(0, (int)(xMax - xMin)));
    float bestDist = 0.0f;

    for (int attempt = 0; attempt < 6; attempt++) {
        float cand = xMin + (float)(RandRange(0, (int)(xMax - xMin)));
        float minDist = 9999.0f;
        for (int ci = 0; ci < MAX_CUBES; ci++) {
            if (!cubes[ci].isActive || ci == slot) continue;
            float dx = fabsf(cand - cubes[ci].position.x);
            if (dx < minDist) minDist = dx;
        }
        if (minDist > bestDist) { bestDist = minDist; bestX = cand; }
    }

    // ── Velocidade: percurso do topo (y=78) até a zona de cartas ─────────────────
    float travelDist = (float)(SCREEN_HEIGHT - 155) - 78.0f;
    float timeWindow = (boss.currentStage == 1) ? CUBE_TIME_S1 :
                       (boss.currentStage == 2) ? CUBE_TIME_S2 : CUBE_TIME_S3;
    float spd = (travelDist / timeWindow) * 1.12f;  // +12% para cruzar a linha

    // Popula o slot
    cubes[slot].prop          = p;
    cubes[slot].correctOption = newCorrect;
    cubes[slot].numOptions    = n;
    cubes[slot].isActive      = 1;
    cubes[slot].answered      = 0;
    cubes[slot].playerChoice  = -1;
    cubes[slot].wasCorrect    = 0;
    cubes[slot].position      = (Vector2){ bestX, 78.0f };  // nasce no topo
    cubes[slot].fallSpeed     = spd;
    cubes[slot].totalTime     = timeWindow;
    cubes[slot].timeLeft      = timeWindow;
    cubes[slot].resultTimer   = 0.0f;

    // Trigger arm summon animation on boss
    boss.attackTimer = 0.55f;
}

// ==================================================================================
//                              UPDATE
// ==================================================================================

// Aplica resultado de uma resposta do jogador
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
        globalScore += 100 * boss.currentStage;
        // Shield block animation at the cube's current position
        boss.shieldTimer = 0.85f;
        boss.shieldPos   = cubes[cubeIdx].position;
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
        if (boss.playerHP <= 0) {
            boss.playerHP = 0;
            boss.phase    = BPHASE_DEFEAT;
        } else {
            boss.phase      = BPHASE_PLAYER_HIT;
            boss.phaseTimer = 0.35f;
            boss.shakeTimer = 0.35f;
        }
    }
}

// Aplica timeout (cubo saiu da tela sem resposta)
static void ApplyTimeout(int cubeIdx) {
    cubes[cubeIdx].answered    = 1;
    cubes[cubeIdx].playerChoice = -1;  // timeout
    cubes[cubeIdx].wasCorrect  = 0;
    cubes[cubeIdx].resultTimer = 0.9f;

    feedbackCube  = cubeIdx;
    feedbackTimer = 0.9f;

    boss.playerHP -= PLAYER_DAMAGE;
    if (boss.playerHP <= 0) {
        boss.playerHP = 0;
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

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (boss.phase != BPHASE_VICTORY && boss.phase != BPHASE_DEFEAT) {
            gamePaused = true;
        } else {
            currentScreen = SCREEN_TITLE;
        }
        return;
    }

    if (boss.phase == BPHASE_FIGHTING || boss.phase == BPHASE_BOSS_HIT || boss.phase == BPHASE_PLAYER_HIT) {
        globalTimer += dt;
    }

    // ── INTRO ────────────────────────────────────────────────────────────────────
    if (boss.phase == BPHASE_INTRO) {
        boss.phaseTimer -= dt;
        if (boss.phaseTimer <= 0.0f) boss.phase = BPHASE_FIGHTING;
        return;
    }

    // ── FIM DE JOGO ──────────────────────────────────────────────────────────────
    if (boss.phase == BPHASE_VICTORY) {
        if (IsKeyPressed(KEY_ENTER)) currentScreen = SCREEN_TITLE;
        return;
    }
    if (boss.phase == BPHASE_DEFEAT) {
        if (IsKeyPressed(KEY_ENTER)) currentScreen = SCREEN_TITLE;  // Volta ao lobby
        return;
    }

    // ── PAUSA PÓS-ACERTO/ERRO ────────────────────────────────────────────────────
    if (boss.phase == BPHASE_BOSS_HIT || boss.phase == BPHASE_PLAYER_HIT) {
        boss.phaseTimer -= dt;
        boss.shakeTimer -= dt;
        if (boss.phaseTimer <= 0.0f) boss.phase = BPHASE_FIGHTING;
        // Não return: ainda atualiza o feedbackTimer e cubos abaixo
    }

    // ── ATUALIZA ESTÁGIO ─────────────────────────────────────────────────────────
    if (boss.bossHP > BOSS_HP_STAGE2)      boss.currentStage = 1;
    else if (boss.bossHP > BOSS_HP_STAGE3) boss.currentStage = 2;
    else                                    boss.currentStage = 3;

    boss.spawnCooldown = (boss.currentStage == 1) ? SPAWN_RATE_S1 :
                          (boss.currentStage == 2) ? SPAWN_RATE_S2 : SPAWN_RATE_S3;

    // ── FEEDBACK TIMER ───────────────────────────────────────────────────────────
    if (feedbackTimer > 0.0f) {
        feedbackTimer -= dt;
        if (feedbackTimer <= 0.0f) feedbackCube = -1;
    }

    // ── ATTACK & SHIELD TIMERS ───────────────────────────────────────────────────
    if (boss.attackTimer > 0.0f) boss.attackTimer -= dt;
    if (boss.shieldTimer > 0.0f) boss.shieldTimer -= dt;

    // ── SPAWN ────────────────────────────────────────────────────────────────────
    if (boss.phase == BPHASE_FIGHTING) {
        boss.spawnTimer -= dt;
        if (boss.spawnTimer <= 0.0f) {
            boss.spawnTimer = boss.spawnCooldown;
            SpawnCube();
        }
    }

    // ── SELECIONA CUBO MAIS URGENTE (mais baixo, sem resposta) ──────────────────
    int lowestSlot = -1;
    float lowestY  = -1.0f;
    for (int i = 0; i < MAX_CUBES; i++) {
        if (cubes[i].isActive && !cubes[i].answered) {
            if (cubes[i].position.y > lowestY) {
                lowestY    = cubes[i].position.y;
                lowestSlot = i;
            }
        }
    }

    // Atualiza layout dos cartões ao trocar de cubo ativo
    if (lowestSlot != selectedCube) {
        selectedCube = lowestSlot;
        if (selectedCube >= 0)
            UpdateCardLayout(cubes[selectedCube].numOptions);
        else
            numActiveCards = 0;
    }

    // ── INPUT DO MOUSE ───────────────────────────────────────────────────────────
    Vector2 mouse = GetMousePosition();
    hoveredCard = -1;

    // Só aceita clique enquanto não está em fase de pausa
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

    // ── UPDATE CUBOS ─────────────────────────────────────────────────────────────
    for (int i = 0; i < MAX_CUBES; i++) {
        if (!cubes[i].isActive) continue;

        if (cubes[i].answered) {
            cubes[i].resultTimer -= dt;
            if (cubes[i].resultTimer <= 0.0f) cubes[i].isActive = 0;
            continue;
        }

        // Cai em direção ao jogador
        cubes[i].position.y += cubes[i].fallSpeed * dt;
        cubes[i].timeLeft   -= dt;

        // Timeout: cubo chega à zona de resposta sem ser respondido
        if (cubes[i].timeLeft <= 0.0f || cubes[i].position.y > CARD_ZONE_Y - 30) {
            ApplyTimeout(i);
        }
    }
}

// ==================================================================================
//                         DRAW — BOSS EQUAL
// ==================================================================================

// ── Helper: raio elétrico entre dois pontos ─────────────────────────────────────────
static void DrawLightningBolt(Vector2 from, Vector2 to, float t, float seed,
                               float thickness, Color col) {
    float dx = to.x - from.x, dy = to.y - from.y;
    float len = sqrtf(dx*dx + dy*dy);
    if (len < 1.0f) return;
    // vetor perpendicular normalizado
    float px = -dy / len, py = dx / len;
    int segs = 9;
    Vector2 prev = from;
    for (int i = 1; i <= segs; i++) {
        float prog = (float)i / segs;
        float bx   = from.x + dx * prog;
        float by   = from.y + dy * prog;
        // zigzag temporal com seed para múltiplos raios independentes
        float amp  = len * 0.13f * sinf(prog * 3.14159f);
        float zz   = sinf(t * 42.0f + i * 1.9f + seed * 3.7f) * amp;
        Vector2 curr = { bx + px * zz, by + py * zz };
        DrawLineEx(prev, curr, thickness, col);
        prev = curr;
    }
    DrawLineEx(prev, to, thickness, col);
}

// ── Pixel-art robot character "Equal" ────────────────────────────────────────────
static void DrawBossEqual(float t, int stage) {
    // State flags
    int isAngry     = (boss.phase == BPHASE_BOSS_HIT || stage == 3);
    int isAttacking = (boss.attackTimer > 0.0f);

    // Idle bob + horizontal shake on hit
    float bobY = sinf(t * 2.2f) * 4.5f;
    float cx   = SCREEN_WIDTH / 2.0f;
    if (boss.shakeTimer > 0.0f && boss.phase == BPHASE_BOSS_HIT)
        cx += sinf(t * 85.0f) * 9.0f * (boss.shakeTimer / 0.4f);

    float cy = 132.0f + bobY;  // head center Y

    // ── Color palette ─────────────────────────────────────────────────────────────
    Color headCol   = (Color){ 18, 20, 30, 255 };
    Color bodyWhite = (Color){ 220, 224, 235, 255 };
    Color bodyGray  = (Color){ 140, 146, 162, 255 };
    Color darkGray  = (Color){ 58,  63,  80, 255 };
    Color suitMid   = (Color){ 180, 185, 198, 255 };
    Color earColor  = (Color){ 65,  210, 230, 255 };
    Color earDark   = (Color){ 32,  140, 162, 255 };
    Color eyeNormal = (Color){ 50,  220, 238, 255 };
    Color eyeAngry  = (Color){ 230,  42,  42, 255 };
    Color eyeColor  = isAngry ? eyeAngry : eyeNormal;
    Color mouthCol  = isAngry ? (Color){210, 36, 36, 255} : eyeNormal;
    Color techLine  = isAngry ? (Color){200, 60, 60, 255} : COLOR_NEON_CYAN;

    // Ambient glow colour under feet
    Color ambGlow = isAngry ? COLOR_NEON_RED :
                   (stage == 2) ? COLOR_NEON_GOLD : COLOR_NEON_CYAN;

    // ── SHADOW (ellipse on the floor) ─────────────────────────────────────────────
    DrawEllipse((int)cx, (int)(cy + 128), 42, 9, ColorAlpha(BLACK, 0.28f));

    // ── AMBIENT AURA ──────────────────────────────────────────────────────────────
    float aura = 0.06f + sinf(t * 2.5f) * 0.02f;
    DrawCircle((int)cx, (int)cy, 80, ColorAlpha(ambGlow, aura));

    // ══ EARS (cyan triangular pixel-art style) ════════════════════════════════════
    // Left ear
    DrawTriangle(
        (Vector2){ cx - 26.0f, cy - 32.0f },
        (Vector2){ cx - 52.0f, cy - 70.0f },
        (Vector2){ cx - 10.0f, cy - 68.0f },
        earColor);
    DrawTriangle(  // dark inner triangle
        (Vector2){ cx - 30.0f, cy - 36.0f },
        (Vector2){ cx - 48.0f, cy - 64.0f },
        (Vector2){ cx - 18.0f, cy - 62.0f },
        earDark);
    // Right ear
    DrawTriangle(
        (Vector2){ cx + 26.0f, cy - 32.0f },
        (Vector2){ cx + 10.0f, cy - 68.0f },
        (Vector2){ cx + 52.0f, cy - 70.0f },
        earColor);
    DrawTriangle(  // dark inner
        (Vector2){ cx + 30.0f, cy - 36.0f },
        (Vector2){ cx + 18.0f, cy - 62.0f },
        (Vector2){ cx + 48.0f, cy - 64.0f },
        earDark);

    // ══ HEAD (large dark circle) ══════════════════════════════════════════════════
    DrawCircle((int)cx, (int)cy, 40, headCol);
    DrawCircleLines((int)cx, (int)cy, 40, (Color){ 48, 54, 72, 255 });
    // Subtle top-left specular highlight
    DrawCircleSector((Vector2){ cx - 10, cy - 12 }, 18.0f,
                     210.0f, 330.0f, 10, ColorAlpha(WHITE, 0.04f));

    // ══ EYES ══════════════════════════════════════════════════════════════════════
    if (!isAngry) {
        // Normal: flat cyan square eyes (pixel-art look)
        DrawRectangle((int)(cx - 24), (int)(cy - 14), 18, 15, eyeColor);
        DrawRectangle((int)(cx +  6), (int)(cy - 14), 18, 15, eyeColor);
        // Glint
        DrawRectangle((int)(cx - 22), (int)(cy - 12),  8,  6, ColorAlpha(WHITE, 0.40f));
        DrawRectangle((int)(cx +  8), (int)(cy - 12),  8,  6, ColorAlpha(WHITE, 0.40f));
        // Blink every ~5.5 s
        if (fmodf(t, 5.5f) < 0.10f) {
            DrawRectangle((int)(cx - 24), (int)(cy - 14), 18, 15, headCol);
            DrawRectangle((int)(cx +  6), (int)(cy - 14), 18, 15, headCol);
        }
    } else {
        // Angry: hollow pulsing red ring eyes + angled brows
        float ep = 1.0f + sinf(t * 14.0f) * 0.14f;
        // Brows
        DrawLineEx((Vector2){ cx - 28, cy - 24 }, (Vector2){ cx -  8, cy - 17 }, 3.5f, eyeColor);
        DrawLineEx((Vector2){ cx +  8, cy - 17 }, (Vector2){ cx + 28, cy - 24 }, 3.5f, eyeColor);
        // Eyes
        DrawCircle((int)(cx - 15), (int)(cy - 6), (int)(12 * ep), ColorAlpha(eyeColor, 0.18f));
        DrawCircleLines((int)(cx - 15), (int)(cy - 6), (int)(12 * ep), eyeColor);
        DrawCircle((int)(cx + 15), (int)(cy - 6), (int)(12 * ep), ColorAlpha(eyeColor, 0.18f));
        DrawCircleLines((int)(cx + 15), (int)(cy - 6), (int)(12 * ep), eyeColor);
    }

    // ══ MOUTH ════════════════════════════════════════════════════════════════════
    float mY = cy + 22.0f;
    if (isAttacking) {
        // Ataque: sobrancelhas raivosas + boca aberta gritando
        float ap = boss.attackTimer / 0.55f;  // 1→0 durante o ataque
        // Sobrancelhas carrancudas
        DrawLineEx((Vector2){ cx - 28, cy - 22 }, (Vector2){ cx - 6,  cy - 15 }, 4.0f,
                   (Color){ 230, 60, 60, 255 });
        DrawLineEx((Vector2){ cx +  6, cy - 15 }, (Vector2){ cx + 28, cy - 22 }, 4.0f,
                   (Color){ 230, 60, 60, 255 });
        // Boca aberta (retângulo que cresce durante o arremesso)
        float openH = 4.0f + (1.0f - ap) * 12.0f;
        DrawRectangleRounded(
            (Rectangle){ cx - 14, mY - 2, 28, openH }, 0.25f, 4,
            (Color){ 180, 25, 25, 255 });
        DrawRectangleRoundedLines(
            (Rectangle){ cx - 14, mY - 2, 28, openH }, 0.25f, 4,
            (Color){ 230, 60, 60, 255 });
    } else if (!isAngry) {
        // Idle: linha reta tensa — expressao neutra/intimidadora
        // Sobrancelhas finas levemente franzidas
        DrawLineEx((Vector2){ cx - 20, cy - 20 }, (Vector2){ cx -  6, cy - 17 }, 2.0f,
                   ColorAlpha(eyeColor, 0.55f));
        DrawLineEx((Vector2){ cx +  6, cy - 17 }, (Vector2){ cx + 20, cy - 20 }, 2.0f,
                   ColorAlpha(eyeColor, 0.55f));
        // Boca: linha fina e tensa (sem sorriso)
        DrawLineEx((Vector2){ cx - 12, mY }, (Vector2){ cx + 12, mY }, 2.8f,
                   ColorAlpha(mouthCol, 0.70f));
    } else {
        // Raiva total: boca serrilhada (como na imagem direita)
        float jH = 8.0f;
        DrawLineEx((Vector2){ cx - 20, mY      }, (Vector2){ cx - 10, mY + jH }, 3.0f, mouthCol);
        DrawLineEx((Vector2){ cx - 10, mY + jH }, (Vector2){ cx,      mY      }, 3.0f, mouthCol);
        DrawLineEx((Vector2){ cx,      mY      }, (Vector2){ cx + 10, mY + jH }, 3.0f, mouthCol);
        DrawLineEx((Vector2){ cx + 10, mY + jH }, (Vector2){ cx + 20, mY      }, 3.0f, mouthCol);
    }

    // ══ NECK ══════════════════════════════════════════════════════════════════════
    float neckY = cy + 40.0f;
    DrawRectangle((int)(cx - 10), (int)neckY, 20, 11, darkGray);

    // ══ TORSO ════════════════════════════════════════════════════════════════════
    float torsoY = neckY + 11.0f;
    // White suit body
    DrawRectangleRounded((Rectangle){ cx - 32, torsoY, 64, 60 }, 0.12f, 4, bodyWhite);
    // Shoulder pads
    DrawRectangle((int)(cx - 40), (int)(torsoY + 2), 12, 20, suitMid);
    DrawRectangle((int)(cx + 28), (int)(torsoY + 2), 12, 20, suitMid);
    // Centre suit panel
    DrawRectangle((int)(cx - 16), (int)(torsoY + 9),  32, 40, suitMid);
    DrawRectangle((int)(cx - 12), (int)(torsoY + 13), 24, 30, darkGray);
    // Pulsing tech lines on panel
    for (int l = 0; l < 4; l++) {
        float ly = torsoY + 17.0f + l * 6.5f;
        float gl = 0.45f + sinf(t * 5.0f + l * 0.9f) * 0.28f;
        DrawLineEx((Vector2){ cx - 8, ly }, (Vector2){ cx + 8, ly }, 1.8f,
                   ColorAlpha(techLine, gl));
    }

    // ══ ARMS ════════════════════════════════════════════════════════════════════
    float armBaseY = torsoY + 16.0f;

    // Braço esquerdo — pose de invocação: levanta em V durante ataque
    float ls       = sinf(t * 1.8f) * 12.0f;
    Vector2 laBase = { cx - 40.0f, armBaseY };
    Vector2 laTip;
    if (isAttacking) {
        float prog  = 1.0f - (boss.attackTimer / 0.55f);
        float raise = sinf(prog * 3.14159f * 0.85f);  // 0→1→0 suave
        laTip = (Vector2){
            laBase.x - 28.0f - 8.0f  * raise,   // abre para fora
            laBase.y - 52.0f * raise              // ergue para cima
        };
    } else {
        laTip = (Vector2){ laBase.x - 16.0f, laBase.y + 36.0f + ls };
    }
    DrawLineEx(laBase, laTip, 14.0f, bodyGray);
    DrawLineEx(laBase, laTip, 10.0f, bodyWhite);
    DrawCircleV(laBase, 9.0f, suitMid);
    DrawCircleV(laTip,  7.0f, darkGray);

    // Braço direito — pose de invocação: levanta em V durante ataque
    Vector2 raBase = { cx + 40.0f, armBaseY };
    Vector2 raTip;
    if (isAttacking) {
        float prog  = 1.0f - (boss.attackTimer / 0.55f);
        float raise = sinf(prog * 3.14159f * 0.85f);
        raTip = (Vector2){
            raBase.x + 28.0f + 8.0f  * raise,   // abre para fora
            raBase.y - 52.0f * raise              // ergue para cima
        };
    } else {
        float rs = -sinf(t * 1.8f) * 12.0f;
        raTip = (Vector2){ raBase.x + 16.0f, raBase.y + 36.0f + rs };
    }
    DrawLineEx(raBase, raTip, 14.0f, bodyGray);
    DrawLineEx(raBase, raTip, 10.0f, bodyWhite);
    DrawCircleV(raBase, 9.0f, suitMid);
    DrawCircleV(raTip,  7.0f, isAttacking ? ColorAlpha(ambGlow, 0.9f) : darkGray);

    // ══ RAIOS ELÉTRICOS (só durante invocação) ══════════════════════════════════════
    if (isAttacking) {
        float prog  = 1.0f - (boss.attackTimer / 0.55f);
        float raise = sinf(prog * 3.14159f * 0.85f);
        if (raise > 0.15f) {
            float alpha = raise;
            Color bolt1 = ColorAlpha(ambGlow, alpha * 0.95f);
            Color bolt2 = ColorAlpha(WHITE,   alpha * 0.55f);
            Color bolt3 = ColorAlpha(ambGlow, alpha * 0.70f);

            // 3 raios sobrepostos para efeito de intensidade
            DrawLightningBolt(laTip, raTip, t,        0.0f, 2.8f, bolt1);
            DrawLightningBolt(laTip, raTip, t + 0.18f, 1.0f, 1.6f, bolt2);
            DrawLightningBolt(laTip, raTip, t + 0.37f, 2.0f, 1.4f, bolt3);

            // Brilho nos punhos
            float glowR = 10.0f + raise * 10.0f;
            DrawCircle((int)laTip.x, (int)laTip.y, (int)(glowR + 6),
                       ColorAlpha(ambGlow, alpha * 0.18f));
            DrawCircle((int)laTip.x, (int)laTip.y, (int)glowR,
                       ColorAlpha(ambGlow, alpha * 0.55f));
            DrawCircle((int)raTip.x, (int)raTip.y, (int)(glowR + 6),
                       ColorAlpha(ambGlow, alpha * 0.18f));
            DrawCircle((int)raTip.x, (int)raTip.y, (int)glowR,
                       ColorAlpha(ambGlow, alpha * 0.55f));

            // Aura elétrica no centro (onde a proposição vai materializar)
            float midX = (laTip.x + raTip.x) / 2.0f;
            float midY = (laTip.y + raTip.y) / 2.0f - 20.0f * raise;
            DrawCircle((int)midX, (int)midY, (int)(22 * raise),
                       ColorAlpha(ambGlow, raise * 0.22f));
            DrawCircleLines((int)midX, (int)midY, (int)(22 * raise),
                            ColorAlpha(WHITE, raise * 0.45f));
        }
    }

    // ══ LEGS ════════════════════════════════════════════════════════════════════
    float legY = torsoY + 60.0f;
    float legS = sinf(t * 1.8f) * 3.5f;
    // Left leg
    DrawRectangle((int)(cx - 28 + legS), (int)legY, 22, 28, bodyWhite);
    DrawRectangle((int)(cx - 32 + legS), (int)(legY + 28), 26, 9, darkGray);
    // Right leg
    DrawRectangle((int)(cx +  6 - legS), (int)legY, 22, 28, bodyWhite);
    DrawRectangle((int)(cx +  6 - legS), (int)(legY + 28), 26, 9, darkGray);

    // ══ HIT FLASH (white overlay) ════════════════════════════════════════════════
    if (boss.phase == BPHASE_BOSS_HIT && boss.shakeTimer > 0.0f) {
        float fa = (boss.shakeTimer / 0.4f) * 0.38f;
        DrawCircle((int)cx, (int)cy, 44, ColorAlpha(WHITE, fa));
        DrawRectangle((int)(cx - 40), (int)torsoY, 80, 68, ColorAlpha(WHITE, fa * 0.65f));
    }

    // ══ NAME TAG ════════════════════════════════════════════════════════════════
    const char *nm = "E Q U A L";
    Color nameCol  = isAngry ? (Color){ 230, 55, 55, 255 } : eyeNormal;
    DrawText(nm, (int)(cx - MeasureText(nm, 13) / 2),
             (int)(legY + 41), 13, ColorAlpha(nameCol, 0.85f));
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

    // Expressão lógica
    int exprSize = 24;
    int exprW = MeasureText(cube->prop->expression, exprSize);
    // Estágios 1-2: centralizado verticalmente (sem prefixo). Estágio 3: topo.
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

    // Prefixo: apenas no estágio 3 (1 e 2 usam o painel de variáveis fixo)
    if (cube->prop->stage == 3) {
        int pfxSize = 11;
        int pfxW = MeasureText(cube->prop->prefix, pfxSize);
        if (pfxW > CUBE_W - 10) pfxSize = 10;
        pfxW = MeasureText(cube->prop->prefix, pfxSize);
        DrawText(cube->prop->prefix, (int)(x+CUBE_W/2-pfxW/2), (int)(y+46),
                 pfxSize, ColorAlpha(COLOR_NEON_PURPLE, 0.80f));
    }

    // Barra de tempo
    float ratio = cube->timeLeft / cube->totalTime;
    if (ratio < 0) ratio = 0;
    Color barCol = ratio > 0.5f ? COLOR_NEON_GREEN :
                   ratio > 0.25f ? COLOR_NEON_GOLD : COLOR_NEON_RED;
    DrawRectangle((int)(x+10), (int)(y+CUBE_H-11), CUBE_W-20, 6,
                  ColorAlpha(COLOR_PANEL_BORDER, 0.8f));
    DrawRectangle((int)(x+10), (int)(y+CUBE_H-11), (int)((CUBE_W-20)*ratio), 6,
                  barCol);

    // ══ EFEITO DE CHOQUE NO SPAWN (primeiros ~0.7s de vida) ═══════════════════
    if (ratio > 0.86f) {
        float sa  = (ratio - 0.86f) / 0.14f;  // 1→0 enquanto o choque some
        float flk = 0.5f + sinf((float)GetTime() * 55.0f) * 0.5f;  // flicker rápido
        Color eCol  = ColorAlpha(COLOR_NEON_CYAN, sa * flk);
        Color eColW = ColorAlpha(WHITE,           sa * flk * 0.45f);

        // Bordas elétricas externas
        DrawRectangleRoundedLines(
            (Rectangle){ x - 5, y - 5, CUBE_W + 10, CUBE_H + 10 },
            0.14f, 6, eCol);
        DrawRectangleRoundedLines(
            (Rectangle){ x - 2, y - 2, CUBE_W + 4, CUBE_H + 4 },
            0.14f, 6, eColW);

        // Farost nos 4 cantos
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
                       (Vector2){ corners[c].x + cosf(ang * DEG2RAD) * spkLen,
                                  corners[c].y + sinf(ang * DEG2RAD) * spkLen },
                       2.0f, ColorAlpha(COLOR_NEON_CYAN, sa * 0.9f));
            DrawCircle((int)corners[c].x, (int)corners[c].y,
                       (int)(5 * sa * flk), ColorAlpha(WHITE, sa * 0.6f));
        }

        // Raio elétrico horizontal no meio do cubo
        Vector2 midL = { x,         y + CUBE_H / 2.0f };
        Vector2 midR = { x + CUBE_W, y + CUBE_H / 2.0f };
        DrawLightningBolt(midL, midR, (float)GetTime(), 3.0f, 1.5f,
                          ColorAlpha(COLOR_NEON_CYAN, sa * 0.70f));
    }
}

// ==================================================================================
//                         DRAW — CARTÕES DE RESPOSTA (MOUSE)
// ==================================================================================

static void DrawAnswerCards(float t) {
    // Determina qual cubo mostrar nos cartões
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

    // Painel de fundo da zona
    Rectangle zone = { 35.0f, (float)(CARD_ZONE_Y - 26),
                        SCREEN_WIDTH - 70.0f, (float)(CARD_H + 48) };
    DrawRectangleRounded(zone, 0.10f, 6, ColorAlpha(COLOR_PANEL_BG, 0.80f));
    DrawRectangleRoundedLines(zone, 0.10f, 6, ColorAlpha(COLOR_GRID_LINE, 0.20f));

    // Label da zona
    const char *zoneLabel = isFeedback ? "Resposta:" : "Clique na opcao correta:";
    int zlw = MeasureText(zoneLabel, 13);
    DrawText(zoneLabel, SCREEN_WIDTH/2 - zlw/2, CARD_ZONE_Y - 20, 13,
             ColorAlpha(COLOR_TEXT_MUTED, 0.75f));

    // Desenha cada cartão
    for (int i = 0; i < numActiveCards; i++) {
        Color accent = cube->prop->accentColor;
        Color bg, border, textCol;

        if (isFeedback) {
            // Mostra resultado: verde = correto, vermelho = errado
            int isCorrect = (i == cube->correctOption);
            int isChosen  = (i == cube->playerChoice);

            if (isCorrect) {
                bg      = ColorAlpha(COLOR_NEON_GREEN, 0.22f);
                border  = COLOR_NEON_GREEN;
                textCol = COLOR_NEON_GREEN;
            } else if (isChosen && !isCorrect) {
                bg      = ColorAlpha(COLOR_NEON_RED, 0.22f);
                border  = COLOR_NEON_RED;
                textCol = COLOR_NEON_RED;
            } else {
                bg      = ColorAlpha(COLOR_PANEL_BG, 0.5f);
                border  = ColorAlpha(COLOR_PANEL_BORDER, 0.3f);
                textCol = ColorAlpha(COLOR_TEXT_MUTED, 0.4f);
            }
        } else {
            int hov  = (i == hoveredCard);
            float pulse = sinf(t * 6.0f) * 0.04f;
            bg      = hov ? ColorAlpha(accent, 0.18f + pulse) : ColorAlpha(COLOR_PANEL_BG, 0.90f);
            border  = hov ? ColorAlpha(accent, 0.90f) : ColorAlpha(COLOR_PANEL_BORDER, 0.55f);
            textCol = hov ? WHITE : COLOR_TEXT_MUTED;

            if (hov) {
                // Halo de hover
                DrawRectangleRoundedLines(
                    (Rectangle){answerCards[i].x-3, answerCards[i].y-3,
                                answerCards[i].width+6, answerCards[i].height+6},
                    0.18f, 6, ColorAlpha(accent, 0.22f + sinf(t*8)*0.09f));
            }
        }

        DrawRectangleRounded(answerCards[i], 0.15f, 6, bg);
        DrawRectangleRoundedLines(answerCards[i], 0.15f, 6, border);

        // Letra da opção (A/B/C/D) no canto
        const char *letter = (i==0)?"A":(i==1)?"B":(i==2)?"C":"D";
        DrawText(letter, (int)(answerCards[i].x + 9), (int)(answerCards[i].y + 7),
                 14, ColorAlpha(accent, isFeedback ? 0.5f :
                                (i == hoveredCard ? 0.9f : 0.45f)));

        // Texto da opção (reduz fonte se necessário)
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

        // DEBUG: mostra qual cartão é o correto — REMOVER antes do build final
        if (!isFeedback && i == cube->correctOption) {
            const char *dbg = "[OK]";
            int dbgW = MeasureText(dbg, 11);
            DrawText(dbg,
                     (int)(answerCards[i].x + answerCards[i].width - dbgW - 6),
                     (int)(answerCards[i].y + answerCards[i].height - 18),
                     11, ColorAlpha(COLOR_NEON_GREEN, 0.70f));
        }

        // Ícone de correto/errado no feedback
        if (isFeedback) {
            if (i == cube->correctOption) {
                DrawText("V", (int)(answerCards[i].x + answerCards[i].width - 22),
                         (int)(answerCards[i].y + 8), 16, COLOR_NEON_GREEN);
            } else if (i == cube->playerChoice) {
                DrawText("X", (int)(answerCards[i].x + answerCards[i].width - 22),
                         (int)(answerCards[i].y + 8), 16, COLOR_NEON_RED);
            }
        }
    }
}

// ==================================================================================
//                         DRAW — ESCUDO (resposta correta)
// ==================================================================================

static void DrawPlayerShield(float t) {
    if (boss.shieldTimer <= 0.0f) return;

    float alpha = boss.shieldTimer / 0.85f;       // Fade out
    float scale = 1.0f + (1.0f - alpha) * 0.4f;  // Expand as it fades

    // Use cube position or fallback to player area
    float sx = boss.shieldPos.x;
    float sy = boss.shieldPos.y;

    Color sc  = COLOR_NEON_CYAN;
    float r   = 72.0f * scale;

    // Outer glow rings
    DrawCircle((int)sx, (int)sy, (int)(r + 18), ColorAlpha(sc, alpha * 0.06f));
    DrawCircle((int)sx, (int)sy, (int)(r + 10), ColorAlpha(sc, alpha * 0.12f));
    DrawCircle((int)sx, (int)sy, (int)r,         ColorAlpha(sc, alpha * 0.22f));
    DrawCircleLines((int)sx, (int)sy, (int)r,    ColorAlpha(sc, alpha));
    DrawCircleLines((int)sx, (int)sy, (int)(r - 5), ColorAlpha(sc, alpha * 0.45f));

    // Hex-style inner lines for shield feel
    for (int i = 0; i < 6; i++) {
        float ang  = i * 60.0f * DEG2RAD + t * 1.2f;
        float ang2 = (i + 1) * 60.0f * DEG2RAD + t * 1.2f;
        Vector2 p1 = { sx + cosf(ang)  * (r - 8), sy + sinf(ang)  * (r - 8) };
        Vector2 p2 = { sx + cosf(ang2) * (r - 8), sy + sinf(ang2) * (r - 8) };
        DrawLineEx(p1, p2, 2.0f, ColorAlpha(sc, alpha * 0.55f));
    }

    // Bold tick / checkmark in centre
    float hx = sx, hy = sy;
    DrawLineEx((Vector2){ hx - 20, hy + 2  }, (Vector2){ hx -  6, hy + 18 }, 5.0f,
               ColorAlpha(COLOR_NEON_GREEN, alpha));
    DrawLineEx((Vector2){ hx -  6, hy + 18 }, (Vector2){ hx + 22, hy - 16 }, 5.0f,
               ColorAlpha(COLOR_NEON_GREEN, alpha));

    // "BLOQUEADO!" text
    const char *txt = "BLOQUEADO!";
    int tw = MeasureText(txt, 20);
    DrawText(txt, (int)(sx - tw / 2), (int)(sy - r - 26), 20,
             ColorAlpha(COLOR_NEON_GREEN, alpha));
}

// ==================================================================================
//                         DRAW — JOGADOR
// ==================================================================================

static void DrawPlayer(float t) {
    float px  = SCREEN_WIDTH / 2.0f;
    float py  = (float)CARD_ZONE_Y - 72.0f;
    float bob = sinf(t * 2.2f) * 3.5f;
    Color pc  = (boss.phase == BPHASE_PLAYER_HIT) ? COLOR_NEON_RED : COLOR_NEON_CYAN;

    // Anel de levitação
    DrawEllipse((int)px, (int)(py+26+bob), 24, 6, ColorAlpha(pc, 0.18f));
    DrawEllipseLines((int)px, (int)(py+26+bob), 24, 6, ColorAlpha(pc, 0.45f));

    // Corpo
    DrawRectangle((int)(px-10),(int)(py-6+bob), 20, 24, ColorAlpha(pc,0.22f));
    DrawRectangleLines((int)(px-10),(int)(py-6+bob), 20, 24, pc);

    // Cabeça
    DrawCircle((int)px,(int)(py-20+bob), 12.0f, ColorAlpha(pc,0.18f));
    DrawCircleLines((int)px,(int)(py-20+bob), 12.0f, pc);
    DrawCircle((int)px,(int)(py-20+bob), 6.0f, pc);

    // Braço + seta
    DrawLineEx((Vector2){px+10,py-2+bob}, (Vector2){px+30,py-15+bob}, 3.0f, pc);
    DrawTriangle((Vector2){px+30,py-21+bob},(Vector2){px+36,py-13+bob},
                 (Vector2){px+24,py-11+bob}, pc);
    DrawCircle((int)px,(int)(py-20+bob),18.0f,ColorAlpha(pc,0.06f));
}

// ==================================================================================
//                         DRAW — HUD
// ==================================================================================

static void DrawPlayerHearts(void) {
    float hy = (float)CARD_ZONE_Y - 40.0f;
    DrawText("JOGADOR", 20, (int)(hy - 16), 13, COLOR_TEXT_MUTED);
    for (int i = 0; i < PLAYER_MAX_HP; i++) {
        Color col = (i < boss.playerHP) ? COLOR_NEON_GREEN
                                        : ColorAlpha(COLOR_TEXT_MUTED, 0.22f);
        float hx = 20.0f + i * 38.0f;
        DrawCircle((int)(hx+6),  (int)(hy+6),  8.0f, col);
        DrawCircle((int)(hx+18), (int)(hy+6),  8.0f, col);
        DrawTriangle((Vector2){hx,hy+10},(Vector2){hx+24,hy+10},
                     (Vector2){hx+12,hy+24}, col);
        if (i < boss.playerHP)
            DrawCircle((int)(hx+12),(int)(hy+10),14.0f,ColorAlpha(col,0.12f));
    }
}

static void DrawBossHPBar(void) {
    int barH = 260, barW = 26;
    int bx = SCREEN_WIDTH - 52;
    int by = (SCREEN_HEIGHT - barH) / 2;

    DrawRectangle(bx, by, barW, barH, ColorAlpha(COLOR_PANEL_BG, 0.75f));
    DrawRectangleLines(bx, by, barW, barH, ColorAlpha(COLOR_NEON_RED, 0.35f));

    float ratio  = (float)boss.bossHP / BOSS_MAX_HP;
    Color barCol = ratio > 0.66f ? COLOR_NEON_GREEN :
                   ratio > 0.33f ? COLOR_NEON_GOLD  : COLOR_NEON_RED;
    int fillH = (int)(barH * ratio);
    DrawRectangle(bx, by + barH - fillH, barW, fillH, ColorAlpha(barCol, 0.85f));

    for (int i = 0; i < 7; i++) {
        float midY = by + (float)barH / 7 * i + (float)barH / 14;
        int   filled = (midY > (by + barH - fillH));
        DrawTriangle((Vector2){(float)bx+2, midY-9},
                     (Vector2){(float)bx+2, midY+9},
                     (Vector2){(float)bx+barW-2, midY},
                     filled ? ColorAlpha(barCol, 0.9f) : ColorAlpha(COLOR_TEXT_MUTED, 0.18f));
    }
    DrawText("EQUAL", bx-5, by-22, 14, COLOR_TEXT_MUTED);
    DrawText(TextFormat("%d%%", boss.bossHP), bx, by+barH+5, 13, ColorAlpha(barCol,0.85f));
}

// ==================================================================================
//                    DRAW — PAINEL DE VARIÁVEIS (legenda fixa, topo-esquerdo)
// ==================================================================================

static void DrawParamLegend(float t) {
    Rectangle panel = { 15.0f, 52.0f, 192.0f, 78.0f };

    // Fundo glass + borda animada
    DrawRectangleRounded(panel, 0.16f, 6, ColorAlpha(COLOR_PANEL_BG, 0.92f));
    DrawRectangleRoundedLines(panel, 0.16f, 6,
        ColorAlpha(COLOR_NEON_CYAN, 0.26f + sinf(t * 3.0f) * 0.10f));
    // Linha de acento no topo
    DrawLineEx(
        (Vector2){ panel.x + 8,               panel.y + 1.0f },
        (Vector2){ panel.x + panel.width - 8,  panel.y + 1.0f },
        1.5f, ColorAlpha(COLOR_NEON_CYAN, 0.55f));

    // Título
    DrawText("VARIAVEIS:", (int)(panel.x + 10), (int)(panel.y + 7), 11,
             ColorAlpha(COLOR_TEXT_MUTED, 0.68f));

    // ── P = Verdadeiro ───────────────────────────────────────────────────────────
    float ry1 = panel.y + 26.0f;
    DrawText("P", (int)(panel.x + 10), (int)ry1, 20, COLOR_NEON_CYAN);
    DrawText("=", (int)(panel.x + 28), (int)(ry1 + 2), 14,
             ColorAlpha(COLOR_TEXT_MUTED, 0.60f));
    DrawRectangleRounded(
        (Rectangle){ panel.x + 44, ry1, 136.0f, 21.0f }, 0.30f, 4,
        ColorAlpha(COLOR_NEON_GREEN, 0.14f));
    DrawRectangleRoundedLines(
        (Rectangle){ panel.x + 44, ry1, 136.0f, 21.0f }, 0.30f, 4,
        ColorAlpha(COLOR_NEON_GREEN, 0.45f));
    DrawText("Verdadeiro", (int)(panel.x + 50), (int)(ry1 + 5), 12, COLOR_NEON_GREEN);
    DrawText("(V)",  (int)(panel.x + 156), (int)(ry1 + 5), 12,
             ColorAlpha(COLOR_NEON_GREEN, 0.72f));

    // ── Q = Falso ─────────────────────────────────────────────────────────────────
    float ry2 = panel.y + 52.0f;
    DrawText("Q", (int)(panel.x + 10), (int)ry2, 20, COLOR_NEON_CYAN);
    DrawText("=", (int)(panel.x + 28), (int)(ry2 + 2), 14,
             ColorAlpha(COLOR_TEXT_MUTED, 0.60f));
    DrawRectangleRounded(
        (Rectangle){ panel.x + 44, ry2, 136.0f, 21.0f }, 0.30f, 4,
        ColorAlpha(COLOR_NEON_RED, 0.14f));
    DrawRectangleRoundedLines(
        (Rectangle){ panel.x + 44, ry2, 136.0f, 21.0f }, 0.30f, 4,
        ColorAlpha(COLOR_NEON_RED, 0.45f));
    DrawText("Falso",  (int)(panel.x + 50), (int)(ry2 + 5), 12, COLOR_NEON_RED);
    DrawText("(F)",   (int)(panel.x + 156), (int)(ry2 + 5), 12,
             ColorAlpha(COLOR_NEON_RED, 0.72f));
}

// ==================================================================================
//                              DRAW PRINCIPAL
// ==================================================================================

void DrawBossScreen(void) {
    float t = (float)GetTime();

    ClearBackground(COLOR_BG_DARK);
    DrawThemeGrid(SCREEN_WIDTH, SCREEN_HEIGHT, CELL_SIZE);

    // Pulso vermelho (HP crítico)
    if (boss.playerHP == 1)
        DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,
                      ColorAlpha(COLOR_NEON_RED, 0.05f + sinf(t*4.5f)*0.03f));

    if (boss.phase == BPHASE_BOSS_HIT)
        DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,
                      ColorAlpha(COLOR_NEON_GREEN, 0.07f));
    if (boss.phase == BPHASE_PLAYER_HIT)
        DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,
                      ColorAlpha(COLOR_NEON_RED, 0.15f));

    // ── Elementos do jogo ────────────────────────────────────────────────────────
    DrawParamLegend(t);          // Painel de variáveis P/Q (canto superior esquerdo)
    DrawBossEqual(t, boss.currentStage);

    for (int i = 0; i < MAX_CUBES; i++) {
        if (cubes[i].isActive)
            DrawLogicCube(&cubes[i], (i == selectedCube), t);
    }

    DrawPlayerShield(t);   // Shield block effect (above cubes, below player)
    DrawPlayer(t);
    DrawAnswerCards(t);

    // ── HUD Topo ─────────────────────────────────────────────────────────────────
    const char *stgStr = (boss.currentStage == 1) ? "ESTAGIO 1: INICIANTE" :
                          (boss.currentStage == 2) ? "ESTAGIO 2: AVANCADO"  :
                                                     "ESTAGIO 3: CRITICO!";
    DrawUnifiedHUD("FASE 3: BOSS FIGHT", stgStr, "Clique com o MOUSE nas respostas para rebater os ataques");

    // ── HUD Inferior ─────────────────────────────────────────────────────────────
    Rectangle botHud = { 15, SCREEN_HEIGHT - 35, SCREEN_WIDTH - 30, 28 };
    DrawThemeGlassPanel(botHud, 0.20f, ColorAlpha(COLOR_GRID_LINE, 0.15f));
    const char *hint = (selectedCube >= 0 && feedbackTimer <= 0.0f)
        ? "Use o MOUSE para clicar na resposta   |   Responda antes do tempo!"
        : "Aguarde...   |   ESC: Menu";
    int hw = MeasureText(hint, 13);
    DrawText(hint, SCREEN_WIDTH/2 - hw/2, SCREEN_HEIGHT - 28, 13, COLOR_TEXT_MUTED);

    DrawPlayerHearts();
    DrawBossHPBar();
    DrawThemeVignette(SCREEN_WIDTH, SCREEN_HEIGHT);

    // ── Overlays ─────────────────────────────────────────────────────────────────
    if (boss.phase == BPHASE_INTRO) {
        DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ColorAlpha(BLACK,0.62f));
        const char *it = "M E E T I N G   E Q U A L";
        DrawText(it, SCREEN_WIDTH/2 - MeasureText(it,40)/2, SCREEN_HEIGHT/2-35, 40,
                 ColorAlpha(COLOR_NEON_CYAN, 0.6f + sinf(t*3.5f)*0.3f));
        const char *is = "Clique na equivalencia logica correta para repelir os cubos!";
        DrawText(is, SCREEN_WIDTH/2 - MeasureText(is,16)/2,
                 SCREEN_HEIGHT/2 + 22, 16, COLOR_TEXT_MUTED);
        const char *is2 = "O cursor do mouse e sua unica arma.";
        DrawText(is2, SCREEN_WIDTH/2 - MeasureText(is2,15)/2,
                 SCREEN_HEIGHT/2 + 48, 15, ColorAlpha(COLOR_TEXT_MUTED,0.65f));
    }

    if (boss.phase == BPHASE_VICTORY) {
        DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ColorAlpha(BLACK,0.70f));
        const char *wt = "EQUAL DERROTADA!";
        DrawText(wt, SCREEN_WIDTH/2-MeasureText(wt,56)/2, SCREEN_HEIGHT/2-72, 56,
                 COLOR_NEON_GREEN);
        const char *ws = "Os sistemas foram restaurados com sucesso!";
        DrawText(ws, SCREEN_WIDTH/2-MeasureText(ws,20)/2, SCREEN_HEIGHT/2, 20,
                 COLOR_TEXT_MUTED);
        const char *sc = TextFormat("PONTUACAO FINAL: %d", globalScore);
        DrawText(sc, SCREEN_WIDTH/2-MeasureText(sc,26)/2, SCREEN_HEIGHT/2+36, 26,
                 COLOR_NEON_GOLD);
        if (sinf(t*3.0f) > 0.0f) {
            const char *et = "Pressione ENTER para voltar ao Menu";
            DrawText(et, SCREEN_WIDTH/2-MeasureText(et,18)/2,
                     SCREEN_HEIGHT/2+80, 18, COLOR_TEXT_MUTED);
        }
    }

    if (boss.phase == BPHASE_DEFEAT) {
        DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ColorAlpha(BLACK,0.78f));
        const char *lt = "SISTEMA COMPROMETIDO";
        DrawText(lt, SCREEN_WIDTH/2-MeasureText(lt,50)/2, SCREEN_HEIGHT/2-70, 50,
                 COLOR_NEON_RED);
        const char *ls = "Equal tomou controle total dos sistemas!";
        DrawText(ls, SCREEN_WIDTH/2-MeasureText(ls,20)/2, SCREEN_HEIGHT/2+4, 20,
                 COLOR_TEXT_MUTED);
        if (sinf(t*3.0f) > 0.0f) {
            const char *rt = "Pressione ENTER para tentar novamente";
            DrawText(rt, SCREEN_WIDTH/2-MeasureText(rt,18)/2,
                      SCREEN_HEIGHT/2+50, 18, COLOR_TEXT_MUTED);
        }
    }
    DrawPhaseBanner();
}
