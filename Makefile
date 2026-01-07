CC := gcc
CFLAGS := -g
OBJS := dyn-string sexp sexp-parser interpreter test-generic
OBJS := $(foreach obj,$(OBJS), obj/$(obj).o)

.PHONY: all
all: main

main: src/main.c $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

test-sexp: src/test-sexp.c $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

run-test-sexp: test-sexp
	./$<

test-interpreter: src/test-interpreter.c $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

run-test-interpreter: test-interpreter
	./$<

$(OBJS): obj/%.o: src/%.c
	$(CC) -c $(CFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -f main test-sexp $(OBJS)
