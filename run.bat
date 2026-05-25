@echo off
if exist logic_rush.exe (
    logic_rush.exe
) else (
    echo Compilando Logic Rush...
    make run
    if errorlevel 1 (
        echo.
        echo ERRO: Falha na compilacao. Verifique se w64devkit e Raylib estao instalados.
        echo Veja a secao "Como Rodar" no README.md para instrucoes.
        pause
    )
)
