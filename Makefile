CC := gcc
CFLAGS := -g
C_INCLUDE_PATH := inc
OBJS := sexp dyn-string
OBJS := $(foreach obj,$(OBJS), obj/$(obj).o)

.PHONY: all
all: main

main: src/main.c obj/sexp.o obj/dyn-string.o
	$(CC) $(CFLAGS) $^ -o $@

test-sexp: src/test-sexp.c obj/sexp.o obj/dyn-string.o
	$(CC) $(CFLAGS) $^ -o $@

run-test-sexp: test-sexp
	./$>

$(OBJS): obj/%.o: src/%.c
	$(CC) -c $(CFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -f main test-sexp obj/sexp.o obj/dyn-string.o
