@echo off
REM Logic Rush — Inicia o jogo E o Placar de Lideres ao mesmo tempo
REM O viewer roda em janela separada e se atualiza automaticamente.

REM Compilar tudo se necessario
if not exist logic_rush.exe (
    echo Compilando Logic Rush...
    make all
    if errorlevel 1 (
        echo.
        echo ERRO: Falha na compilacao. Verifique se w64devkit e Raylib estao instalados.
        echo Veja a secao "Como Rodar" no README.md para instrucoes.
        pause
        exit /b 1
    )
) else if not exist ranking_viewer.exe (
    echo Compilando ranking viewer...
    make ranking_viewer.exe
    if errorlevel 1 (
        echo.
        echo AVISO: Nao foi possivel compilar o ranking viewer. Iniciando so o jogo.
    )
)

REM Abre o viewer em processo separado (nao bloqueia)
if exist ranking_viewer.exe (
    start "Logic Rush - Placar de Lideres" ranking_viewer.exe
)

REM Inicia o jogo (bloqueia ate fechar)
logic_rush.exe
