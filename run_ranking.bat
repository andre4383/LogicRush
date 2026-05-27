@echo off
REM Logic Rush — Abre o Placar de Lideres em janela separada
REM O placar se atualiza automaticamente sempre que uma partida termina.

if exist ranking_viewer.exe (
    ranking_viewer.exe
) else (
    echo Compilando ranking viewer...
    make ranking_viewer.exe
    if errorlevel 1 (
        echo.
        echo ERRO: Falha na compilacao. Verifique se w64devkit e Raylib estao instalados.
        echo Veja a secao "Como Rodar" no README.md para instrucoes.
        pause
    ) else (
        ranking_viewer.exe
    )
)
