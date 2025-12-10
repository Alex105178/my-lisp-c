CC := gcc
CFLAGS := -g

.PHONY: all
all: main

main: main.c
	$(CC) $(CFLAGS) main.c -o main

.PHONY: clean
clean:
	rm main
