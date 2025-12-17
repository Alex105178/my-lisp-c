#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../inc/dyn-string.h"
#include "../inc/sexp.h"
#include "../inc/sexp-parser.h"

struct String* test(char* input) {
    struct ParseRes res = parse_sexp(input);
    if (res.error == PE_NONE) {
        struct String *string = string_alloc(2);
        string_sexp(res.val.good.sexp, string);
        return string;
    } else {
        return NULL;
    }
}

void run_tests(char* list[][3], int len) {
    int num_success = 0;
    int num_fail = 0;
    for (int i = 0; i < len; i = i + 1) {
        char** entry = list[i];
        if (0 == strcmp(entry[0], "==")) {
            struct String* result = test(entry[1]);
            if (result == NULL) {
                printf("Test %d failed.\n", i);
                num_fail += 1;                
            } else if (0 == strcmp(result->mem, entry[2])) {
                num_success += 1;
            } else {
                num_fail += 1;
            }
        } else {
            fprintf(stderr, "Error, invalid compare operator!");
            assert(false);
        }
    }
    printf("Success: %d, Fail: %d, Total: %d\n", num_success, num_fail, len);
}

char* test_list[][3] = {
    {"==", "foo", "foo"},
    {"==", " foo ", "foo"},
    {"==", "(foo)", "(foo)"},
    {"==", "( foo )", "(foo)"},
    {"==", "(foo bar baz)", "(foo bar baz)"},
    {"==", "( foo   bar\n  baz  )", "(foo bar baz)"},
    {"==", "(foo (bar baz))", "(foo (bar baz))"},
    {"==", "( foo   ( bar  baz  ) )", "(foo (bar baz))"},
    {"==", "(foo . (bar baz))", "(foo bar baz)"},
    {"==", "( foo  .   ( bar  baz  ) )", "(foo bar baz)"},
    {"==", "(foo . bar)", "(foo . bar)"},
    {"==", "( foo  .  bar )", "(foo . bar)"},
};
int test_list_len = sizeof(test_list) / sizeof(test_list[0]);

int main(int argc, char* argv[]) {
    run_tests(test_list, test_list_len);
}
