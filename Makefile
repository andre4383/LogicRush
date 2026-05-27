CFLAGS = -Wall -Wextra -std=c99

# ── Configuração por Sistema Operacional ─────────────────────────────────────────
ifeq ($(OS), Windows_NT)
    W64DEV_GCC = C:/raylib/w64devkit/bin/gcc.exe
    ifeq ($(wildcard $(W64DEV_GCC)),$(W64DEV_GCC))
        CC = $(W64DEV_GCC)
    else
        CC = gcc
    endif

    # ── Auto-detect Raylib ────────────────────────────────────────────────────────
    # Ordem: pkg-config → paths comuns → RAYLIB_PATH manual → erro
    _PKGCONFIG_OK := $(shell pkg-config --exists raylib 2>/dev/null && echo yes || echo no)

    ifeq ($(_PKGCONFIG_OK),yes)
        CFLAGS += $(shell pkg-config --cflags raylib 2>/dev/null)
        LDFLAGS  = $(shell pkg-config --libs raylib 2>/dev/null)
    else
        _RL_SEARCH = \
            C:/raylib/raylib/src \
            C:/msys64/mingw64/include \
            C:/msys2/mingw64/include \
            C:/msys64/ucrt64/include \
            C:/msys2/ucrt64/include \
            C:/mingw64/include
        ifdef RAYLIB_PATH
            _RL_SEARCH += $(RAYLIB_PATH)/src $(RAYLIB_PATH)/include $(RAYLIB_PATH)
        endif

        _RL_INC := $(firstword $(foreach d,$(_RL_SEARCH),$(if $(wildcard $(d)/raylib.h),$(d),)))

        ifeq ($(_RL_INC),)
            $(error Raylib nao encontrado. Instale via MSYS2: pacman -S mingw-w64-x86_64-raylib  OU  defina: set RAYLIB_PATH=<caminho>)
        endif

        # Deriva lib dir: paths /include -> /lib, outros (ex: /src) ficam iguais
        _RL_LIB := $(if $(filter %/include,$(_RL_INC)),$(patsubst %/include,%/lib,$(_RL_INC)),$(_RL_INC))

        CFLAGS  += -I$(_RL_INC)
        LDFLAGS  = -L$(_RL_LIB) -lraylib -lopengl32 -lgdi32 -lwinmm
    endif

    TARGET  = logic_rush.exe
    RUN_CMD = ./$(TARGET)
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S), Darwin)
        ifeq ($(shell uname -m), arm64)
            BREW = /opt/homebrew
        else
            BREW = /usr/local
        endif
        CC      = clang
        CFLAGS += -I$(BREW)/include
        LDFLAGS = -L$(BREW)/lib -lraylib \
                  -framework OpenGL -framework Cocoa \
                  -framework IOKit -framework CoreVideo
    else
        CC      = gcc
        LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    endif
    TARGET  = logic_rush
    RUN_CMD = ./$(TARGET)
endif

# ── Fontes e Objetos ─────────────────────────────────────────────────────────────
SRC = $(wildcard src/core/*.c)           \
      $(wildcard src/menu/*.c)           \
      $(wildcard src/fase_quiz/*.c)       \
      $(wildcard src/fase_labirinto/*.c) \
      $(wildcard src/fase_boss/*.c)       \
      $(wildcard src/ranking/*.c)
# Exclude viewer-only files from main game build
SRC := $(filter-out src/ranking_viewer/%,$(SRC))

OBJ = $(SRC:.c=.o)

# ── Regras ────────────────────────────────────────────────────────────────────────
all: $(TARGET) ranking_viewer$(_EXE)

_EXE = $(if $(filter Windows_NT,$(OS)),.exe,)

ranking_viewer$(_EXE): src/ranking/ranking_viewer.o src/ranking_viewer/ranking_viewer_main.o
	$(CC) src/ranking/ranking_viewer.o src/ranking_viewer/ranking_viewer_main.o -o $@ $(LDFLAGS)

# Special rule: compile ranking.c as a standalone for viewer (no game.h deps)
src/ranking/ranking_viewer.o: src/ranking/ranking.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	$(RUN_CMD)

run_viewer: ranking_viewer$(_EXE)
	$(if $(filter Windows_NT,$(OS)),ranking_viewer.exe,./ranking_viewer)

# Clean funciona nos 3 SOs (rm disponível no w64devkit e Git Bash)
clean:
	rm -f $(OBJ) $(TARGET) ranking_viewer$(_EXE) src/ranking/ranking_viewer.o src/ranking_viewer/*.o

.PHONY: all clean run run_viewer
