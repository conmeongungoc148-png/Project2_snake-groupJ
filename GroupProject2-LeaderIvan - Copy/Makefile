CC = gcc
CFLAGS = -Wall -O2 -std=c99 -Isrc -Iraylib/include
LDFLAGS = -Lraylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm

SRC_DIR = src
SRC = $(SRC_DIR)/main.c $(SRC_DIR)/game.c $(SRC_DIR)/snake.c $(SRC_DIR)/food.c
OBJ = $(SRC:.c=.o)
RANDOM_ID := $(shell echo %RANDOM%)
EXEC = snake_game_$(RANDOM_ID).exe

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/*.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /Q src\*.o snake_game*.exe
