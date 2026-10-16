CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
INCLUDES = -Isrc

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
TARGET = chess

# Detection de l'OS
ifeq ($(OS),Windows_NT)
    TARGET := chess.exe
    LDFLAGS := -lraylib -lgdi32 -lwinmm
    RM = rm -f
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        LDFLAGS := -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    else
        LDFLAGS := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    endif
    RM = rm -f
endif

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	-$(RM) src/*.o $(TARGET)

.PHONY: all clean
