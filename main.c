#include <stdio.h>

#include "dyn-string.h"
#include "sexp.h"

int main(int argc, char *argv[]) {
    struct ParseRes res = parse_sexp(argv[1]);
    if (res.error == PE_NONE) {
        struct String *string = string_alloc(2);
        string_sexp(res.val.good.sexp, string);
        printf("%s\n", string->mem);
    } else {        
        printf("ERROR, %s, at position %ld:\n%s\n",
               parse_error_to_str(res.error),
               res.val.bad.loc - argv[1],
               res.val.bad.loc);
    }
}
