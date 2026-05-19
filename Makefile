CC = clang
CFLAGS = -Wall -Wextra -std=c99

# Auto-detect Homebrew path on macOS
ifeq ($(shell uname), Darwin)
    ifeq ($(shell uname -m), arm64)
        BREW_PATH = /opt/homebrew
    else
        BREW_PATH = /usr/local
    endif
    CFLAGS += -I$(BREW_PATH)/include
    LDFLAGS = -L$(BREW_PATH)/lib -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
else
    # Linux fallback
    LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif

# Source files from structured subdirectories
SRC = $(wildcard src/core/*.c) $(wildcard src/menu/*.c) $(wildcard src/fase_labirinto/*.c)
OBJ = $(SRC:.c=.o)
TARGET = logic_rush

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/core/*.o src/menu/*.o src/fase_labirinto/*.o $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
