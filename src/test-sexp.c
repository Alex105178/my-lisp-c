#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "../inc/dyn-string.h"
#include "../inc/sexp.h"
#include "../inc/sexp-parser.h"
#include "../inc/test-generic.h"

struct String* test_sexp(char* input) {
    struct ParseRes res = parse_sexp(input);
    if (res.error == PE_NONE) {
        struct String *string = string_alloc(2);
        string_sexp(res.val.good.sexp, string);
        return string;
    } else {
        return string_from_cstr("TEST_ERROR");
    }
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
    {"==", "(.)", "TEST_ERROR"},
    {"==", "( . )", "TEST_ERROR"},
    {"==", "(. foo)", "TEST_ERROR"},
    {"==", "( .  foo )", "TEST_ERROR"},
    {"==", "(foo .)", "TEST_ERROR"},
    {"==", "( foo  . )", "TEST_ERROR"},
    {"==", "(foo . bar baz)", "TEST_ERROR"},
    {"==", "( foo  .  bar  baz )", "TEST_ERROR"},
};

int test_list_len = sizeof(test_list) / sizeof(test_list[0]);

int main(int argc, char* argv[]) {
    run_tests(test_list, test_list_len, test_sexp);
}
