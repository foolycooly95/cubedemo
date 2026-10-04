CC = gcc
CFLAGS = -Wall -std=c99 $(shell pkg-config --cflags sdl3)
LDFLAGS = $(shell pkg-config --libs sdl3) -lm

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
TARGET = cubedemo

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: run clean
