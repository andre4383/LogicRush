CFLAGS = -Wall -Wextra -std=c99

# ── Configuração por Sistema Operacional ─────────────────────────────────────────
ifeq ($(OS), Windows_NT)
    RAYLIB_PATH ?= C:/raylib/raylib
    W64DEV_GCC   = C:/raylib/w64devkit/bin/gcc.exe

    ifeq ($(wildcard $(W64DEV_GCC)),$(W64DEV_GCC))
        CC = $(W64DEV_GCC)
    else
        CC = gcc
    endif

    CFLAGS += -I$(RAYLIB_PATH)/src
    LDFLAGS = -L$(RAYLIB_PATH)/src -lraylib -lopengl32 -lgdi32 -lwinmm
    TARGET  = logic_rush.exe
    RUN_CMD = $(TARGET)
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
      $(wildcard src/fase_boss/*.c)

OBJ = $(SRC:.c=.o)

# ── Regras ────────────────────────────────────────────────────────────────────────
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	$(RUN_CMD)

# Clean funciona nos 3 SOs (rm disponível no w64devkit e Git Bash)
clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean run
