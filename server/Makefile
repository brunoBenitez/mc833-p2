CC = gcc
CC_FLAGS = -std=gnu11 -ggdb -pedantic -Wall -I..

all: server.x
.PHONY: all

server.x: main.c database.o server.o
	$(CC) $(CC_FLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CC_FLAGS) -c $< -o $@

clean:
	rm -f *.o server.x
.PHONY: clean