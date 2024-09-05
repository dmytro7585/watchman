# Компілятор та опції
CC = gcc
CFLAGS = -Os -std=c11 -flto -g -Wall -Wextra -Werror
LDFLAGS = -flto

# Ціль за замовчуванням
all: watchman

# Компіляція програми
watchman: watchman.o
	$(CC) $(LDFLAGS) -o watchman watchman.o

# Компіляція об'єктного файлу
watchman.o: watchman.c
	$(CC) $(CFLAGS) -c watchman.c -o watchman.o

# Очищення
clean:
	rm -f watchman watchman.o
