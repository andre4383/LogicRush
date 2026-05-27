#ifndef DIALOGUE_H
#define DIALOGUE_H

#include "../core/screens.h"
#include "raylib.h"
#include <stdbool.h>

// ── IDs dos locutores ─────────────────────────────────────────────────────────
typedef enum {
    SPEAKER_ASTRO,   // Usa dialoguebox_player.png
    SPEAKER_EQUAL,   // Usa dialoguebox_equal.png
    SPEAKER_SYSTEM   // Sem caixa — apenas texto centralizado (typing animation)
} SpeakerType;

// ── Uma linha de diálogo ──────────────────────────────────────────────────────
#define MAX_DIALOGUE_TEXT 400
typedef struct {
    SpeakerType speaker;
    char text[MAX_DIALOGUE_TEXT];
} DialogueLine;

// ── Sequências numeradas usadas pelo código de história ───────────────────────
typedef enum {
    DSEQ_INTRO = 0,       // 3 linhas de abertura do jogo
    DSEQ_PRE_FASE1,       // Pré-fase 1
    DSEQ_POST_FASE1,      // Pós-fase 1
    DSEQ_PRE_FASE2,       // Pré-fase 2
    DSEQ_POST_FASE2,      // Pós-fase 2
    DSEQ_BOSS_OPEN,       // Abertura da luta final
    DSEQ_BOSS_MID,        // HP médio (<=66)
    DSEQ_BOSS_LOW,        // HP baixo (<=33)
    DSEQ_BOSS_DEFEAT,     // Equal derrotada
    DSEQ_COUNT
} DialogueSeqID;

// ── Animação de digitação ("typing") ─────────────────────────────────────────
#define MAX_TYPING_TEXT 512
typedef struct {
    char   buffer[MAX_TYPING_TEXT];  // Texto revelado até agora
    char   target[MAX_TYPING_TEXT];  // Texto completo
    int    length;                   // Comprimento do target
    float  timer;                    // Tempo acumulado
    float  charDelay;                // Segundos por caractere
    bool   done;
} TypingState;

// ── API pública ───────────────────────────────────────────────────────────────

// Carrega texturas das dialogue boxes (chame uma vez em InitGame)
void Dialogue_Init(void);

// Descarrega texturas
void Dialogue_Unload(void);

// Inicia uma sequência pelo ID.
// nextScreen: tela para onde ir quando a sequência terminar
void Dialogue_StartSeq(DialogueSeqID seqId, GameScreen nextScreen);

// Atualiza o estado da sequência ativa (detecta Enter → avança linha)
void Dialogue_Update(void);

// Renderiza a caixa de diálogo sobre o conteúdo existente
void Dialogue_Draw(void);

// Retorna true enquanto uma sequência estiver ativa
bool Dialogue_IsActive(void);

// ── API de typing animation ───────────────────────────────────────────────────
void Typing_Start(TypingState *s, const char *text, float charDelay);
void Typing_Update(TypingState *s, float dt);
// Retorna texto revelado até agora
const char *Typing_Text(const TypingState *s);
bool Typing_IsDone(const TypingState *s);
// Pula para o fim instantaneamente
void Typing_Skip(TypingState *s);

#endif // DIALOGUE_H
