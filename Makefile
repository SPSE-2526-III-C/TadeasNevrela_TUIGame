CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Iinclude

SRC := $(shell find src -name '*.c')
OBJ := $(SRC:.c=.o)
BIN := tuigame

.PHONY: all clean run

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(OBJ) $(BIN)
