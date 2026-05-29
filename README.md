# Logic Rush

Logic Rush é um jogo rápido focado em desafios constantes de equivalência lógica.
Os jogadores assumem o papel de um inventor enfrentando sua própria criação: uma IA com falhas chamada Equal.

## A história

Em um mundo movido pela inovação em inteligência artificial, o projeto “Equal” deveria ser uma obra-prima da tecnologia com comportamento humano.
Porém, um erro crítico na programação do sistema de equivalência lógica fez com que ela passasse a enxergar o mundo de forma distorcida.

## Visão Geral da Gameplay

A experiência é inspirada em “party games” de alta intensidade e conta com um sistema de dificuldade cíclico, que fica mais difícil a cada loop concluído.

# Mini-Games:

### Cartas da Memória:

- Vire cartas para encontrar símbolos equivalentes antes que o tempo acabe.

### Circuitos Emaranhados (Labirinto):

- Navegue por caminhos bloqueados por portas lógicas. Resolva rapidamente as equações para avançar, ou perca o jogo.

### Enfrentando Equal (Boss Fight):

- Encare a IA diretamente enquanto ela lança expressões lógicas complexas contra você. Simplifique essas expressões usando cubos para reparar os dados e refletir o dano de volta nela.

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

## A equipe

### PROGRAMADORES

- Afonso Araujo
- Lucas Gabriel
- André Montenegro
- Pedro Lima
- Igor Aragão
- Vitor Emmanuel
- Williams Pontes

### DIRETOR DE ARTE

- Breno Gabriel

### TRILHA SONORA

- André Montenegro -> Sound cloud>: https://on.soundcloud.com/5YUaTmZ4KEe4fumDuo
- Gabriel Alves -> Sound cloud: https://soundcloud.com/prodcreativowav  Spotify: https://open.spotify.com/intl-pt/artist/4V48gcTOCJZYrWPtnzd77Q?si=WSN_B0UQSCisZaTPpDnicQ
