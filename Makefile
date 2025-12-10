CC := gcc
CFLAGS := -g

.PHONY: all
all: main

main: main.c sexp.o dyn-string.o
	$(CC) $(CFLAGS) $^ -o $@

test-sexp: test-sexp.c sexp.o dyn-string.o
	$(CC) $(CFLAGS) $^ -o $@

run-test-sexp: test-sexp
	./$>

sexp.o: sexp.c
	$(CC) -c $(CFLAGS) $^ -o $@

dyn-string.o: dyn-string.c
	$(CC) -c $(CFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm main main.o sexp.o dyn-string.o
