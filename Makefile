CC = gcc
CFLAGS = -Os -std=c11 -flto -g -Wall -Wextra -Werror
LDFLAGS = -flto

all: watchman

watchman: watchman.o
	$(CC) $(LDFLAGS) -o watchman watchman.o

watchman.o: watchman.c
	$(CC) $(CFLAGS) -c watchman.c -o watchman.o

clean:
	rm -f watchman watchman.o
