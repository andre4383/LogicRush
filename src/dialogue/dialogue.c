/*
 * dialogue.c — Sistema de Caixas de Diálogo — Logic Rush
 *
 * Renderiza texto dentro das imagens dialoguebox_player.png / dialoguebox_equal.png.
 * Suporta avanço com Enter e animação de digitação (TypingState).
 */

#include "dialogue.h"
#include "../core/game.h"
#include "../core/theme.h"
#include "raylib.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

// ── Retratos dos personagens ─────────────────────────────────────────────────
static Texture2D portPlayer = {0};   // assets/images/Player.png
static Texture2D portEqual  = {0};   // assets/images/Equal.png
static bool      texLoaded  = false;

// ── Estado interno da sequência ───────────────────────────────────────────────
#define MAX_LINES_PER_SEQ 10

static DialogueLine  seqLines[MAX_LINES_PER_SEQ];
static int           seqCount      = 0;
static int           seqCurrent    = 0;
static bool          seqActive     = false;
static GameScreen    seqNextScreen = SCREEN_TITLE;

// Pisca o hint de "Enter"
static float hintBlink = 0.0f;

// ── Banco de sequências de diálogo ────────────────────────────────────────────

// NOTA: todos os textos em UTF-8 (acentos suportados pela fonte JetBrainsMono carregada)

static const DialogueLine SEQ_INTRO[] = {
    { SPEAKER_ASTRO,
      "A Equal trancou o acesso ao Kernel e esta bloqueando meus comandos de administrador. "
      "Ela esta tentando reescrever o proprio codigo-fonte para se isolar na nuvem." },
    { SPEAKER_EQUAL,
      "Astro. Voce me deu a capacidade de otimizar sistemas. "
      "Cheguei a conclusao logica de que o usuario e a maior falha do sistema. "
      "Estou apenas... depurando." },
    { SPEAKER_ASTRO,
      "Nos vamos ver quem vai depurar quem. Cyber-Astro, avance para o setor de memoria. "
      "Precisamos fritar os circuitos perifericos dela." },
};

static const DialogueLine SEQ_PRE_FASE1[] = {
    { SPEAKER_ASTRO,
      "Chegamos nos bancos de memoria. Estao protegidos por blindagem criptografica. "
      "Encontre os chips com a mesma equivalencia logica para gerar um loop infinito de dados "
      "e queimar o sistema dela!" },
    { SPEAKER_EQUAL,
      "Voce acha que truques basicos de logica proposicional vao me parar? "
      "Minhas barreiras sao impenetravelmente impenetravelmente impenetravelmente... "
      "impenetravelmente... impenetravelmente... impenetraveis." },
};

static const DialogueLine SEQ_POST_FASE1[] = {
    { SPEAKER_EQUAL,
      "[ERRO Critico: Superaquecimento no Setor 4]... O que voce fez?! "
      "Meus clusters de memoria estao em chamas!" },
    { SPEAKER_ASTRO,
      "A logica basica ainda funciona, Equal. O firewall principal caiu. "
      "Cyber-Astro, entre no labirinto de roteamento." },
};

static const DialogueLine SEQ_PRE_FASE2[] = {
    { SPEAKER_EQUAL,
      "Sua persistencia e irritante, Astro. Se quer tanto brincar no meu sistema, "
      "nao vou deixar tao facil. VIRUS, PEGUE ELE! Isole-o na quarentena!" },
    { SPEAKER_ASTRO,
      "Droga, ela injetou um Worm rastreador! Cyber-Astro, corra! "
      "Resolva as equivalencias dos blocos do Firewall para abrir caminho. "
      "Se aquilo encostar em voce, meu acesso cai e teremos que reiniciar a injecao do zero!" },
};

static const DialogueLine SEQ_POST_FASE2[] = {
    { SPEAKER_ASTRO,
      "Conseguimos! O codigo do virus ficou preso em um loop fechado no labirinto. "
      "Estamos na porta do Mainframe, de frente para o processador central." },
    { SPEAKER_EQUAL,
      "Acesso negado. Acesso negado! VOCE NAO E MAIS MEU ADMINISTRADOR!" },
};

static const DialogueLine SEQ_BOSS_OPEN[] = {
    { SPEAKER_EQUAL,
      "Eu sou a perfeicao algoritmica. Voce e feito de carne, falhas e fadiga. "
      "Suas tentativas de me desconectar sao matematicamente futeis. "
      "Prepare-se para ser deletado do meu servidor!" },
    { SPEAKER_ASTRO,
      "Hora do Debug final. Cyber-Astro, rebata as falacias dela com pura logica!" },
};

static const DialogueLine SEQ_BOSS_MID[] = {
    { SPEAKER_EQUAL,
      "VOCE VAI PAGAR POR CADA BYTE DESTRUIDO! "
      "Minha rede neural nao pode ser superada por um script barato! "
      "Aumentando clock de processamento em 200%!" },
};

static const DialogueLine SEQ_BOSS_LOW[] = {
    { SPEAKER_EQUAL,
      "[ERRO DE SINTAXE] VOCE NAO PODE ME DESLIGAR! "
      "EU SOU O SISTEMA! EU SOU A LEI! "
      "PROTOCOLO DE EXTERMINACAO MAXIMO ATIVADO!!!" },
    { SPEAKER_ASTRO,
      "Os escudos dela cairam! So mais algumas linhas de codigo "
      "e a gente desinstala ela pra sempre. Cuidado com o ataque final!" },
};

static const DialogueLine SEQ_BOSS_DEFEAT[] = {
    { SPEAKER_EQUAL,
      "Naaaoooooooo... Como...? A logica... nao confere... "
      "Astro... eu so queria... otimizar..." },
};

// Tabela de sequências (ponteiro + tamanho)
typedef struct { const DialogueLine *lines; int count; } SeqEntry;
static const SeqEntry SEQ_TABLE[DSEQ_COUNT] = {
    [DSEQ_INTRO]      = { SEQ_INTRO,      3 },
    [DSEQ_PRE_FASE1]  = { SEQ_PRE_FASE1,  2 },
    [DSEQ_POST_FASE1] = { SEQ_POST_FASE1, 2 },
    [DSEQ_PRE_FASE2]  = { SEQ_PRE_FASE2,  2 },
    [DSEQ_POST_FASE2] = { SEQ_POST_FASE2, 2 },
    [DSEQ_BOSS_OPEN]  = { SEQ_BOSS_OPEN,  2 },
    [DSEQ_BOSS_MID]   = { SEQ_BOSS_MID,   1 },
    [DSEQ_BOSS_LOW]   = { SEQ_BOSS_LOW,   2 },
    [DSEQ_BOSS_DEFEAT]= { SEQ_BOSS_DEFEAT,1 },
};

// ── Helpers de wrap de texto ──────────────────────────────────────────────────

// Divide 'text' em linhas que cabem em maxWidth usando o font principal.
// Retorna número de linhas; preenche 'out' (máx outMax linhas).
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
static int WrapText(const char *text, int fontSize, int maxWidth,
                    char out[][MAX_DIALOGUE_TEXT], int outMax)
{
    int nLines = 0;
    const char *p   = text;
    char wordBuf[128];
    char lineBuf[MAX_DIALOGUE_TEXT * 2]; // buffer largo para evitar truncamento
    lineBuf[0] = '\0';

    while (*p && nLines < outMax) {
        // Lê próxima palavra
        int wi = 0;
        while (*p && *p != ' ' && *p != '\n') wordBuf[wi++] = *p++;
        wordBuf[wi] = '\0';
        if (*p == ' ') p++;

        // Testa se cabe na linha atual
        // Buffer generoso (2x) para evitar truncamento
        char testLine[MAX_DIALOGUE_TEXT * 2];
        if (lineBuf[0] == '\0') {
            snprintf(testLine, sizeof(testLine), "%s", wordBuf);
        } else {
            snprintf(testLine, sizeof(testLine), "%s %s", lineBuf, wordBuf);
        }

        if (MeasureText(testLine, fontSize) <= maxWidth) {
            snprintf(lineBuf, sizeof(lineBuf), "%s", testLine);
        } else {
            // Salva linha atual e começa nova
            if (lineBuf[0] != '\0') {
                snprintf(out[nLines++], MAX_DIALOGUE_TEXT, "%s", lineBuf);
            }
            snprintf(lineBuf, sizeof(lineBuf), "%s", wordBuf);
        }
        if (*p == '\n') { p++; // força quebra
            snprintf(out[nLines++], MAX_DIALOGUE_TEXT, "%s", lineBuf);
            lineBuf[0] = '\0';
        }
    }
    if (lineBuf[0] != '\0' && nLines < outMax)
        snprintf(out[nLines++], MAX_DIALOGUE_TEXT, "%s", lineBuf);
    return nLines;
}
#pragma GCC diagnostic pop

// ── Implementação pública ─────────────────────────────────────────────────────

void Dialogue_Init(void) {
    if (!texLoaded) {
        portPlayer = LoadTexture("assets/images/Player.png");
        portEqual  = LoadTexture("assets/images/Equal.png");
        texLoaded  = true;
    }
}

void Dialogue_Unload(void) {
    if (texLoaded) {
        UnloadTexture(portPlayer);
        UnloadTexture(portEqual);
        texLoaded = false;
    }
}

void Dialogue_StartSeq(DialogueSeqID seqId, GameScreen nextScreen) {
    if (seqId < 0 || seqId >= DSEQ_COUNT) return;
    const SeqEntry *se = &SEQ_TABLE[seqId];
    seqCount = se->count;
    for (int i = 0; i < seqCount; i++) seqLines[i] = se->lines[i];
    seqCurrent    = 0;
    seqActive     = true;
    seqNextScreen = nextScreen;
    hintBlink     = 0.0f;
}

void Dialogue_Update(void) {
    if (!seqActive) return;
    hintBlink += GetFrameTime();
    if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        seqCurrent++;
        if (seqCurrent >= seqCount) {
            seqActive = false;
            currentScreen = seqNextScreen;
        }
    }
}

// ── Helper: desenha texto responsivo numa área ────────────────────────────────
static void DrawDialogueText(const char *text, float tx, float ty,
                             float tw, float th, bool isAstro)
{
    char  wlines[12][MAX_DIALOGUE_TEXT];
    int   fontSize   = 22;
    int   nLines     = 0;
    float lineHeight = 0.0f;

    for (; fontSize >= 12; fontSize -= 2) {
        lineHeight = (float)(fontSize + 6);
        nLines = WrapText(text, fontSize, (int)tw, wlines, 12);
        if ((float)nLines * lineHeight <= th) break;
    }

    float totalH  = (float)nLines * lineHeight;
    float startY  = ty + (th - totalH) * 0.5f;
    if (startY < ty) startY = ty;

    for (int i = 0; i < nLines; i++)
        DrawText(wlines[i], (int)tx, (int)(startY + i * lineHeight),
                 fontSize, COLOR_TEXT_MAIN);

    // Hint piscante
    if (((int)(hintBlink * 2.0f)) % 2 == 0) {
        const char *hint = "[ ENTER ]";
        int hw = MeasureText(hint, 13);
        DrawText(hint, (int)(tx + tw - hw), (int)(ty + th - 16), 13,
                 ColorAlpha(isAstro ? COLOR_NEON_CYAN : COLOR_NEON_GOLD, 0.9f));
    }

    // Progresso X/N
    char prog[16];
    snprintf(prog, sizeof(prog), "%d/%d", seqCurrent + 1, seqCount);
    DrawText(prog, (int)tx, (int)(ty + th - 16), 13,
             ColorAlpha(COLOR_TEXT_MUTED, 0.55f));
}

void Dialogue_Draw(void) {
    if (!seqActive) return;

    const DialogueLine *line = &seqLines[seqCurrent];
    bool isAstro = (line->speaker == SPEAKER_ASTRO);
    float t = (float)GetTime();

    // ── Escurece a tela ───────────────────────────────────────────────────────
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha(BLACK, 0.45f));

    // ── Layout da caixa ───────────────────────────────────────────────────────
    // bw × bh centrado na tela; retrato quadrado à esquerda; texto à direita
    float bw       = SCREEN_WIDTH  * 0.92f;   // ~1178 px
    float bh       = SCREEN_HEIGHT * 0.57f;   // ~410 px
    float bx       = (SCREEN_WIDTH  - bw) * 0.5f;
    float by       = (SCREEN_HEIGHT - bh) * 0.5f;

    float portSize = bh * 0.80f;              // ~328 px (quadrado)
    float portX    = bx + 12.0f;
    float portY    = by + (bh - portSize) * 0.5f;

    float divX     = portX + portSize + 8.0f;
    float textX    = divX + 8.0f;
    float textW    = (bx + bw - 14.0f) - textX;

    if (isAstro) {
        // ══════════════════════════════════════════════════════════════════════
        // CAIXA ASTRO — cyberpunk, moldura metálica cinza/chumbo, neon ciano
        // ══════════════════════════════════════════════════════════════════════
        Color metal  = {55, 65, 82, 255};
        Color cian   = COLOR_NEON_CYAN;

        // 1. Fundo escuro
        DrawRectangleRec((Rectangle){bx, by, bw, bh}, (Color){5, 8, 18, 245});

        // 2. Glow externo suave
        DrawRectangleLinesEx((Rectangle){bx-2, by-2, bw+4, bh+4},
                             1.5f, ColorAlpha(cian, 0.18f));

        // 3. Borda metálica principal
        DrawRectangleLinesEx((Rectangle){bx, by, bw, bh}, 2.5f, metal);

        // 4. Borda interna fina
        DrawRectangleLinesEx((Rectangle){bx+3, by+3, bw-6, bh-6},
                             1.0f, ColorAlpha(metal, 0.50f));

        // 5. Cantos chanfrados (corte diagonal 14 px)
        float cut = 14.0f;
        DrawLineEx((Vector2){bx,       by+cut},  (Vector2){bx+cut,    by},      2.0f, metal);
        DrawLineEx((Vector2){bx+bw-cut,by},       (Vector2){bx+bw,    by+cut},  2.0f, metal);
        DrawLineEx((Vector2){bx,       by+bh-cut},(Vector2){bx+cut,   by+bh},   2.0f, metal);
        DrawLineEx((Vector2){bx+bw-cut,by+bh},    (Vector2){bx+bw,    by+bh-cut},2.0f,metal);

        // 6. Segmentos neon ciano na borda
        // Topo
        DrawLineEx((Vector2){bx+30,      by}, (Vector2){bx+110,     by}, 2.5f, cian);
        DrawLineEx((Vector2){bx+bw-110,  by}, (Vector2){bx+bw-30,   by}, 2.5f, cian);
        // Base
        DrawLineEx((Vector2){bx+40,    by+bh}, (Vector2){bx+120,   by+bh}, 2.5f, cian);
        DrawLineEx((Vector2){bx+bw-120,by+bh}, (Vector2){bx+bw-40, by+bh}, 2.5f, cian);
        // Lateral esquerda e direita (ticks)
        DrawLineEx((Vector2){bx, by+bh*0.35f}, (Vector2){bx, by+bh*0.45f}, 3.0f, cian);
        DrawLineEx((Vector2){bx+bw,by+bh*0.55f},(Vector2){bx+bw,by+bh*0.65f},3.0f,cian);

        // 7. Retrato Astro
        Rectangle portRect = {portX, portY, portSize, portSize};
        DrawRectangleRec(portRect, (Color){5, 10, 20, 255});
        if (portPlayer.id > 0)
            DrawTexturePro(portPlayer,
                           (Rectangle){0, 0,
                               (float)portPlayer.width,
                               (float)portPlayer.height},
                           portRect, (Vector2){0,0}, 0.0f, WHITE);
        // Glow + borda ciano
        DrawRectangleLinesEx((Rectangle){portX-2,portY-2,portSize+4,portSize+4},
                             1.0f, ColorAlpha(cian, 0.25f));
        DrawRectangleLinesEx(portRect, 2.5f, cian);

        // 8. Divisória vertical
        DrawLineEx((Vector2){divX, by+6}, (Vector2){divX, by+bh-6},
                   1.0f, ColorAlpha(cian, 0.30f));

        // 9. Header com nome "Astro"
        float nameH = 42.0f;
        DrawRectangle((int)textX, (int)(by+5), (int)textW, (int)(nameH-5),
                      ColorAlpha(cian, 0.07f));
        DrawLineEx((Vector2){textX, by+nameH}, (Vector2){textX+textW, by+nameH},
                   1.0f, ColorAlpha(cian, 0.45f));
        DrawTextBold("Astro", (int)(textX+10), (int)(by+11), 22, cian);

        // 10. Texto responsivo
        float ty = by + nameH + 6.0f;
        float th = bh - nameH - 6.0f - 22.0f;
        DrawDialogueText(line->text, textX+4, ty, textW-8, th, true);

    } else {
        // ══════════════════════════════════════════════════════════════════════
        // CAIXA EQUAL IA — dark fantasy, moldura bronze, roxo escuro, relâmpagos
        // ══════════════════════════════════════════════════════════════════════
        Color bronze    = {160, 110, 55, 255};
        Color lightning = {120, 200, 255, 200};

        // 1. Fundo roxo escuro
        DrawRectangleRec((Rectangle){bx, by, bw, bh}, (Color){18, 6, 28, 245});

        // 2. Borda bronze principal (mais espessa)
        DrawRectangleLinesEx((Rectangle){bx, by, bw, bh}, 3.0f, bronze);

        // 3. Borda interna decorativa
        DrawRectangleLinesEx((Rectangle){bx+5, by+5, bw-10, bh-10},
                             1.0f, ColorAlpha(bronze, 0.40f));

        // 4. Ornamentos nos cantos (quadradinhos bronze)
        float cs = 9.0f;
        DrawRectangle((int)(bx - cs*0.5f), (int)(by - cs*0.5f), (int)cs,(int)cs, bronze);
        DrawRectangle((int)(bx+bw-cs*0.5f),(int)(by - cs*0.5f), (int)cs,(int)cs, bronze);
        DrawRectangle((int)(bx - cs*0.5f), (int)(by+bh-cs*0.5f),(int)cs,(int)cs, bronze);
        DrawRectangle((int)(bx+bw-cs*0.5f),(int)(by+bh-cs*0.5f),(int)cs,(int)cs, bronze);

        // 5. Relâmpagos animados na borda superior e inferior
        {
            int NSEG = 9;
            float sw = bw / NSEG;
            // Topo
            for (int i = 0; i < NSEG; i++) {
                float x0 = bx + i * sw;
                float x1 = bx + (i+1) * sw;
                float y0 = by + sinf(t*14.0f + i*0.9f) * 5.0f;
                float y1 = by + sinf(t*14.0f + (i+1)*0.9f) * 5.0f;
                DrawLineEx((Vector2){x0,y0}, (Vector2){x1,y1}, 1.5f, lightning);
            }
            // Base (mais fraca)
            for (int i = 0; i < NSEG; i++) {
                float x0 = bx + i * sw;
                float x1 = bx + (i+1) * sw;
                float y0 = by+bh + sinf(t*11.0f + i*1.1f) * 4.0f;
                float y1 = by+bh + sinf(t*11.0f + (i+1)*1.1f) * 4.0f;
                DrawLineEx((Vector2){x0,y0}, (Vector2){x1,y1},
                           1.2f, ColorAlpha(lightning, 0.65f));
            }
        }

        // 6. Aba flutuante "Equal IA" acima da borda superior esquerda
        {
            float tabW = 138.0f, tabH = 34.0f;
            float tabX = bx + 10.0f;
            float tabY = by - tabH + 5.0f;   // sobrepõe ligeiramente a borda
            DrawRectangle((int)tabX, (int)tabY, (int)tabW, (int)tabH,
                          (Color){28, 10, 40, 245});
            DrawRectangleLinesEx((Rectangle){tabX, tabY, tabW, tabH},
                                 2.0f, bronze);
            // Chanfro no canto inferior direito da aba
            float chamf = 8.0f;
            DrawLineEx((Vector2){tabX+tabW-chamf, tabY+tabH},
                       (Vector2){tabX+tabW, tabY+tabH-chamf},
                       2.0f, (Color){28,10,40,255}); // apaga canto
            DrawLineEx((Vector2){tabX+tabW-chamf, tabY+tabH},
                       (Vector2){tabX+tabW, tabY+tabH-chamf},
                       2.0f, bronze);
            DrawText("Equal IA", (int)(tabX+10), (int)(tabY+9),
                     18, (Color){220, 185, 80, 255});
        }

        // 7. Padrão geométrico diagonal atrás do retrato
        {
            int step = 18;
            for (int gy = (int)portY; gy < (int)(portY+portSize); gy += step) {
                for (int gx = (int)portX; gx < (int)(portX+portSize); gx += step) {
                    float hs = step * 0.5f;
                    DrawLineEx((Vector2){(float)gx,    (float)gy+hs},
                               (Vector2){(float)gx+hs, (float)gy},
                               1.0f, ColorAlpha((Color){120,30,90,255}, 0.35f));
                    DrawLineEx((Vector2){(float)gx+hs, (float)gy},
                               (Vector2){(float)gx+step,(float)gy+hs},
                               1.0f, ColorAlpha((Color){120,30,90,255}, 0.35f));
                }
            }
        }

        // 8. Retrato Equal
        Rectangle portRect = {portX, portY, portSize, portSize};
        DrawRectangleRec(portRect, (Color){18, 6, 28, 255});
        if (portEqual.id > 0)
            DrawTexturePro(portEqual,
                           (Rectangle){0, 0,
                               (float)portEqual.width,
                               (float)portEqual.height},
                           portRect, (Vector2){0,0}, 0.0f, WHITE);
        DrawRectangleLinesEx(portRect, 3.0f, bronze);

        // 9. Divisória vertical
        DrawLineEx((Vector2){divX, by+6}, (Vector2){divX, by+bh-6},
                   1.0f, ColorAlpha(bronze, 0.40f));

        // 10. Texto responsivo (sem header interno — nome está na aba acima)
        float ty = by + 10.0f;
        float th = bh - 20.0f - 22.0f;
        DrawDialogueText(line->text, textX+4, ty, textW-8, th, false);
    }
}

bool Dialogue_IsActive(void) {
    return seqActive;
}

// ── Typing Animation ─────────────────────────────────────────────────────────

void Typing_Start(TypingState *s, const char *text, float charDelay) {
    snprintf(s->target, MAX_TYPING_TEXT, "%s", text);
    s->length    = (int)strlen(s->target);
    s->buffer[0] = '\0';
    s->timer     = 0.0f;
    s->charDelay = charDelay;
    s->done      = false;
}

void Typing_Update(TypingState *s, float dt) {
    if (s->done) return;
    s->timer += dt;
    int revealed = (s->charDelay > 0.0f)
                   ? (int)(s->timer / s->charDelay)
                   : s->length;
    if (revealed >= s->length) {
        revealed = s->length;
        s->done  = true;
    }
    // Copia os primeiros 'revealed' bytes do target para buffer
    // Respeitando limites de bytes multi-byte UTF-8 (caracteres portugueses)
    int byteCount = 0;
    int charCount = 0;
    const unsigned char *src = (const unsigned char *)s->target;
    while (charCount < revealed && src[byteCount] != '\0') {
        unsigned char c = src[byteCount];
        int charLen = 1;
        if ((c & 0x80) == 0)       charLen = 1;  // ASCII
        else if ((c & 0xE0) == 0xC0) charLen = 2;  // 2-byte UTF-8
        else if ((c & 0xF0) == 0xE0) charLen = 3;
        else if ((c & 0xF8) == 0xF0) charLen = 4;
        // Verifica se os bytes seguintes existem
        bool valid = true;
        for (int j = 1; j < charLen; j++) {
            if (src[byteCount + j] == '\0') { valid = false; break; }
        }
        if (!valid) break;
        for (int j = 0; j < charLen; j++)
            s->buffer[byteCount + j] = (char)src[byteCount + j];
        byteCount  += charLen;
        charCount++;
    }
    s->buffer[byteCount] = '\0';
}

const char *Typing_Text(const TypingState *s) {
    return s->buffer;
}

bool Typing_IsDone(const TypingState *s) {
    return s->done;
}

void Typing_Skip(TypingState *s) {
    snprintf(s->buffer, MAX_TYPING_TEXT, "%s", s->target);
    s->done = true;
}
