#include <stdio.h>

#include "../inc/dyn-string.h"
#include "../inc/sexp.h"
#include "../inc/sexp-parser.h"
#include "../inc/interpreter.h"

int main(int argc, char *argv[]) {
    struct ParseRes res = parse_sexp(argv[1]);
    if (res.error == PE_NONE) {
        struct String *string = sexp_to_string(res.val.good.sexp);
        string_add_char(string, '\0');
        printf("%s\n", string->mem);
        printf("Evals to: ");
        struct Value* v = eval(res.val.good.sexp, NULL);
        struct String* v_str = value_to_string(v);
        value_free(v);
        string_add_char(v_str, '\0');
        printf("%s\n", v_str->mem);
        sexp_free(res.val.good.sexp);
        string_free(string);
        string_free(v_str);
    } else {        
        printf("ERROR, %s, at position %ld:\n%s\n",
               parse_error_to_str(res.error),
               res.val.bad.loc - argv[1],
               res.val.bad.loc);
    }
}
