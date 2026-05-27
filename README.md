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
