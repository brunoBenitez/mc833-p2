CC = gcc
CC_FLAGS = -std=gnu11 -ggdb -pedantic -Wall -I..

all: client.x
.PHONY: all

client.x: main.c commands.o utils.o client.o
	$(CC) $(CC_FLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CC_FLAGS) -c $< -o $@

clean:
	rm -f *.o client.x
.PHONY: clean