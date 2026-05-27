# Logic Rush
Logic Rush is a fast-paced game centered on constant challenges of logical equivalence
Players take on the role of an inventor facing off against their own creation, a glitched AI named Equal.

## The Story
In a world driven by AI innovation, the "Equal" project was meant to be a masterpiece of life-like technology
however, a critical programming error in her logical equivalence processing caused her to see the world through a distorted lens
Now erratic and unstable, Equal is attempting to seize control of the systems, and you must dive into the code to fix the problem before it's too late

## Gameplay Overview
The experience is inspired by high-energy "party games" and features a cycling difficulty system that gets harder with every completed loop

# Mini-Games:
### Memory Cards:
- Flip cards to find matching logical equivalence symbols before time runs out

### Entangled Circuitry (Maze):
- Navigate a path where progress is blocked by logical gates. Solve the equations quickly to pass, or lose the game

### Meeting Equal (Boss Fight):
- Face the AI directly as she hurls complex logical expressions at you, Simplify these expressions using cubes to repair the data and reflect the damage back at her

## Sistema de Vidas

O jogador começa com **3 vidas** compartilhadas entre todas as fases.

| Fase | Como ganhar vida | Condição |
|------|-----------------|----------|
| Fase 1 — Memory Cards | Acertar **3 pares seguidos** | Cada par deve ser encontrado dentro de 8 s do anterior; errar um par reseta o combo |
| Fase 2 — Entangled Circuitry | Responder **3 portões corretos em sequência** | Cada resposta deve ser dada em até 5 s após a questão aparecer; resposta errada reseta o combo |
| Fase 3 — Meeting Equal | **Não ganha vida** | — |

---

## Placar de Líderes ao Vivo

O Logic Rush inclui um **visualizador de placar independente** que pode rodar em paralelo com o jogo — ideal para eventos, apresentações ou um segundo monitor.

### Como funciona
- O viewer abre em uma janela separada com o placar animado
- Detecta automaticamente quando uma nova partida termina e o `.dat` é atualizado
- Exibe flash **"ATUALIZADO!"** e anima a nova entrada
- Indicador **"AO VIVO"** pulsante confirma que o monitoramento está ativo
- **F11** = tela cheia · **ESC** = fechar

### Como usar no Windows

**Só o placar** (sem o jogo):
```bat
run_ranking.bat
```

**Jogo + placar juntos** (recomendado para eventos):
```bat
run_both.bat
```
O `run_both.bat` compila tudo automaticamente se necessário, abre o viewer numa janela separada e em seguida lança o jogo.

### Como usar no Linux / macOS
```bash
# Compilar tudo (jogo + viewer)
make all

# Abrir o viewer em background e depois o jogo
./ranking_viewer &
make run
```

### Compilar manualmente
```bash
make all          # compila logic_rush + ranking_viewer
make run_viewer   # compila e abre só o viewer
```

---

## Controle (mouse, teclado e 8BitDo)

O jogo aceita **teclado + mouse** e **gamepad** ao mesmo tempo (layout Xbox nos botões). O mapeamento do 8BitDo é carregado de `assets/gamecontrollerdb.txt` na inicialização.

### Pareamento do 8BitDo Ultimate 2C

| Conexão | Configuração |
|---------|----------------|
| **Bluetooth + macOS** | Interruptor traseiro em **D** → segurar o botão de pareamento (estrela) 3 s → parear em Ajustes do Sistema → abrir o jogo |
| **Dongle 2,4 GHz / USB** | Pode usar perfil Xbox (X+Home) se preferir; o Windows/Linux usam as linhas correspondentes no DB |

Pareie o controle **antes** de `make run` quando possível. Se conectar depois, o jogo reaplica o mapeamento automaticamente.

### Tabela de controles

| Ação | Teclado / mouse | Gamepad |
|------|-----------------|---------|
| Mover (labirinto) | WASD / setas | Analógico esquerdo ou D-pad |
| Cursor (menu, cartas, boss) | Mouse | Analógico direito (ou D-pad se o stick direito estiver parado) |
| Selecionar / confirmar | Clique / Enter / Espaço | **A** |
| Voltar / pausar | ESC | **B** ou Start |
| Portão Verdadeiro | V | **Y** |
| Portão Falso | F | **X** |

**Vibração:** feedback ao perder vida e ao concluir uma fase. No **macOS** a vibração pode ser fraca ou ausente.

**Ranking:** digitar o nome continua apenas pelo teclado; **B** ou ESC voltam ao menu.

### Diagnóstico (controle não responde)

```bash
LOGICRUSH_INPUT_DEBUG=1 make run
```

Ou pressione **F12** no jogo. O overlay mostra se cada slot `pad[0..3]` está ativo e o nome do gamepad (ex.: `8BitDo Ultimate 2C`).

**Checklist rápido**

1. LED do controle fixo após parear no Mac.
2. Debug mostra `pad[n]: yes` e nome do 8BitDo.
3. Analógico direito move o cursor ciano no menu.
4. **A** clica em “Iniciar”.
5. No labirinto, o stick esquerdo move o personagem.

---

## Como Rodar

### macOS
```bash
# Instalar Raylib (uma vez)
brew install raylib

# Compilar e executar
make run
```

### Windows
**Pré-requisitos** (instalar uma vez):
1. [w64devkit](https://github.com/skeeto/w64devkit/releases) — extrair em `C:\raylib\w64devkit`
2. [Raylib](https://github.com/raysan5/raylib/releases) — extrair em `C:\raylib\raylib`

**Executar** (via Git Bash, CMD ou terminal do w64devkit):
```bat
make run
```

**Ou** duplo-clique em `run.bat` se o executável já estiver compilado.

**Raylib em outro diretório?** Override via variável de ambiente:
```bat
set RAYLIB_PATH=D:\libs\raylib && make run
```

## The Team
### PROGRAMMERS 
- Afonso Araújo 
- Lucas Gabriel 
- André Montenegro
- Pedro Lima 
- Igor Aragão
- Vitor Emmanuel 
- Williams Pontes

### ART DIRECTOR
- Breno Gabriel
