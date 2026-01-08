#include <stdlib.h>

#include "../inc/sexp-parser.h"
#include "../inc/interpreter.h"
#include "../inc/test-generic.h"

struct String* test_interpreter(char* input) {
    struct ParseRes res = parse_sexp(input);
    if (res.error == PE_NONE) {
        struct String* sexp_str = sexp_to_string(res.val.good.sexp);
        struct Value* v = eval(res.val.good.sexp, NULL);
        struct String* val_str = value_to_string(v);
        value_free(v);
        sexp_free(res.val.good.sexp);
        string_free(sexp_str);
        return val_str;
    } else {        
        return string_from_cstr("PARSE_ERROR");
    }
}

char* test_list[][3] = {
    {"==", "(+ 12 3)", "15"},
    {"==", "(* 12 3)", "36"},
    {"==", "(- 12 3)", "9"},
    {"==", "(/ 12 3)", "4"},
    {"==", "(= 12 3)", "false"},
    {"==", "(= 3 3)", "true"},
    {"==", "(if true 1 0)", "1"},
    {"==", "(if false 1 0)", "0"},
    {"==", "(if (= 3 3) 1 0)", "1"},
    {"==", "(if (= 12 3) 1 0)", "0"},
    {"==", "(let (x 3) (+ x 5))", "8"},
    {"==", "(let (y 5) (let (x 3) (+ x y)))", "8"},
    {"==", "(let (x 5) (let (x 3) (+ x x)))", "6"},
    {"==", "(let (f (lambda (x) (+ x 1))) (f 3))", "4"},
    {"==", "(let (f (let (y 3) (lambda (x) (+ x y)))) (let (y 1) (f 4)))", "7"},
    {"==", "((lambda (n) (+ n 1)) 2)", "3"},
    {
        // Test with fix-point combinator implementing recursive factorial.
        "==",

        "(let (fix (lambda (f) "
                    "((lambda (x) (f (lambda (y) ((x x) y)))) "
                     "(lambda (x) (f (lambda (y) ((x x) y))))))) "
          "(let (fact (lambda (fct) "
                       "(lambda (n) "
                         "(if (= n 0) 1 (* n (fct (- n 1))))))) "
            "(let (factorial (fix fact)) "
              "(factorial 20))))",

        "2432902008176640000"
    }
};

int test_list_len = sizeof(test_list) / sizeof(test_list[0]);

int main(int argc, char* argv[]) {
    run_tests(test_list, test_list_len, test_interpreter);
}
