# 🎨 Guia de Identidade Estética e Design System — Logic Rush

Este guia estabelece os padrões visuais, paleta de cores e componentes reutilizáveis para manter a interface de **Logic Rush** consistente, moderna e com estética cyberpunk/sci-fi premium de alta fidelidade.

Toda a lógica visual foi centralizada no arquivo de cabeçalho [theme.h](file:///Users/andremontenegro/Documents/Faculdade/SegundoPeriodo/LogicRush/src/core/theme.h). Todos os membros do grupo devem importar este arquivo ao criar novas telas, menus ou componentes de gameplay.

---

## 🎨 1. Paleta de Cores e Temas (Color System)

A identidade visual utiliza tons profundos e frios de azul-marinho como plano de fundo (Cyber Cadet), contrastados por realces neon vibrantes e tipografia limpa.

| Nome do Macro | Cor (RGB) | Opacidade | Aplicação Recomendada |
| :--- | :--- | :--- | :--- |
| `COLOR_BG_DARK` | `10, 15, 26` | `255` | Fundo principal da janela / telas. |
| `COLOR_PANEL_BG` | `13, 20, 35` | `255` | Fundo de painéis flutuantes e caixas. |
| `COLOR_PANEL_BORDER`| `20, 30, 50` | `255` | Bordas padrão de painéis e botões inativos. |
| `COLOR_GRID_LINE` | `59, 130, 246` | `255` | Linhas de grade tecnológicas do plano de fundo. |
| `COLOR_NEON_CYAN` | `34, 211, 238` | `255` | Lasers de link, rastros e conexões de rede. |
| `COLOR_NEON_GREEN` | `34, 197, 94` | `255` | Elementos ligados/ativos, jogador, sucesso. |
| `COLOR_NEON_RED` | `239, 68, 68` | `255` | Elementos desligados/inativos, barreiras, erro. |
| `COLOR_NEON_GOLD` | `234, 179, 8` | `255` | Portas lógicas OR, alertas e frisos secundários. |
| `COLOR_NEON_PURPLE` | `168, 85, 247` | `255` | Portas lógicas XOR, portais e núcleos de energia. |
| `COLOR_TEXT_MAIN` | `241, 245, 249` | `255` | Títulos, textos principais e valores de timers. |
| `COLOR_TEXT_MUTED` | `148, 163, 184` | `255` | Legendas, prompts secundários e instruções de tecla. |

---

## 🔤 2. Sistema de Tipografia (Typography System)

Para evitar a fonte bitmap padrão pixelada da Raylib, o Logic Rush utiliza a família de fontes **Inter** (Regular e Bold), renderizada em alta resolução (96px) com suporte a caracteres latinos acentuados.

### A. Substituição Transparente da Raylib
O arquivo [game.h](file:///Users/andremontenegro/Documents/Faculdade/SegundoPeriodo/LogicRush/src/core/game.h) redefine de forma transparente as funções nativas da Raylib utilizando macros associadas a funções inline estáticas de alta fidelidade:

*   `DrawText(const char* text, int x, int y, int size, Color color)`: Desenha texto renderizado com a fonte **Inter Regular**.
*   `MeasureText(const char* text, int size)`: Retorna a largura em pixels do texto desenhado com a fonte **Inter Regular**.
*   `DrawTextBold(const char* text, int x, int y, int size, Color color)`: Desenha texto em negrito utilizando a fonte **Inter Bold**.
*   `MeasureTextBold(const char* text, int size)`: Retorna a largura do texto em negrito.

### B. Suporte a Acentuação em Português
A fonte é carregada em tempo de inicialização no [game.c](file:///Users/andremontenegro/Documents/Faculdade/SegundoPeriodo/LogicRush/src/core/game.c) incluindo um array de codepoints Unicode dinâmico que cobre:
*   ASCII padrão (`32` a `126`)
*   Caracteres acentuados da língua portuguesa (`á, à, â, ã, é, ê, í, ó, ô, õ, ú, ç` e suas versões maiúsculas).

### C. Filtro de Textura Bilinear
Todas as texturas de fontes utilizam `TEXTURE_FILTER_BILINEAR` ativo, garantindo que o redimensionamento do texto (para tamanhos menores ou maiores que 96px) permaneça perfeitamente nítido e suavizado (anti-aliased).

---

## 🛠️ 3. Componentes de Interface Reutilizáveis (Visual Helpers)

Em vez de pintar formas com funções genéricas de desenho da Raylib, utilize as funções embutidas em `theme.h`. Elas encapsulam animações e efeitos matemáticos em tempo real:

### A. Grade Tecnológica (`DrawThemeGrid`)
Desenha linhas finas de circuito cruzadas no fundo do cenário, com reforços estéticos a cada 4 blocos.
```c
// Chame no início do ciclo de desenho (Draw), logo após limpar a tela
DrawThemeGrid(SCREEN_WIDTH, SCREEN_HEIGHT, CELL_SIZE);
```

### B. Painéis de Vidro / Glassmorphism (`DrawThemeGlassPanel`)
Gera caixas de diálogo translúcidas com efeito de desfoque/vidro e borda neon brilhante.
```c
Rectangle hudRect = { 15, 10, SCREEN_WIDTH - 30, CELL_SIZE };
DrawThemeGlassPanel(hudRect, 0.20f, ColorAlpha(COLOR_GRID_LINE, 0.25f));
```

### C. Botões Interativos com Reação a Hover (`DrawThemeButton`)
Gera botões arredondados inteligentes que mudam de cor, iluminam a borda e destacam a cor da fonte quando o cursor do mouse passa por cima.
```c
Rectangle btnPlay = { 100, 200, 200, 50 };
bool mouseOver = CheckCollisionPointRec(GetMousePosition(), btnPlay);
DrawThemeButton(btnPlay, "INICIAR JOGO", 20, mouseOver, COLOR_NEON_CYAN);
```

### D. Linha de Transmissão de Energia/Laser (`DrawThemeLaser`)
Desenha uma linha brilhante pulsante e gera uma aura na extremidade de destino. Ideal para indicar proximidade do jogador com interruptores ou pontos de interesse.
```c
float time = (float)GetTime();
DrawThemeLaser(playerPos, switchPos, time);
```

### E. Fio de Circuito Lógico com Elétrons (`DrawThemeCircuitWire`)
Desenha cabos com dobras em ângulos retos de 90 graus. Quando ativo (`active = true`), gera uma simulação física de **dois elétrons** trafegando simultaneamente para representar fluxo contínuo.
```c
float time = (float)GetTime();
DrawThemeCircuitWire(startPos, endPos, isWireActive, time);
```

### F. Efeito de Vinheta Cinematográfica (`DrawThemeVignette`)
Aplica gradientes de sombra nos cantos da tela, intensificando a imersão e o foco no centro da jogabilidade.
```c
// Chame no final do ciclo de desenho, logo antes de desenhar overlays de vitória/derrota
DrawThemeVignette(SCREEN_WIDTH, SCREEN_HEIGHT);
```

---

## 📏 4. Regras e Boas Práticas Estéticas

1. **Uso de Transparências (`ColorAlpha`)**:
   Nunca desenhe auras ou feixes de luz com opacidade 100%. Sempre adicione transparência (`0.15f` a `0.45f`) para simular reflexão luminosa (glow) realista.
2. **Constantes Dimensionais**:
   Sempre alinhe elementos dinâmicos do cenário à grade de 40px (`CELL_SIZE`) para manter a estrutura harmônica.
3. **Animações Temporais (`GetTime`)**:
   Use ondas senoidais (`sinf(GetTime() * velocidade)`) para modular espessuras de linhas, auras e rotações, evitando elementos estáticos.
