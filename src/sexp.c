#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>

#include "../inc/sexp.h"
#include "../inc/dyn-string.h"

// #define NDEBUG

struct Sexp Nil = {NIL, .val.list = {(void*)NULL, (void*)NULL}};

void string_sexp(struct Sexp *sexp, struct String *string) {
    if (sexp->type == SYM) {
        string_add(string, sexp->val.sym->str);
    } else {
        struct Sexp* cur = sexp;
        string_add(string, "(");
        while (cur != &Nil) {
            if (cur->type == LIST) {
                string_sexp(cur->val.list.car, string);
                if (cur->val.list.cdr != &Nil) {
                    string_add(string, " ");
                }
                cur = cur->val.list.cdr;
            } else {
                string_add(string, ". ");
                string_add(string, cur->val.sym->str);
                cur = &Nil;
            }
        }
        if (cur->type == SYM) {
            
        }
        string_add(string, ")");
    }
}

struct Sexp* sexp_cons(struct Sexp *car, struct Sexp *cdr) {
    struct Sexp *sexp = malloc(sizeof(struct Sexp));
    sexp->type = LIST;
    sexp->val.list.car = car;
    sexp->val.list.cdr = cdr;
    return sexp;
}

/* Creates an s-expression which contains a symbol.
 *   [string] A string representing the symbol itself. [sexp_symbol] takes
 *            complete ownership of [string], and [string] may be freed by
 *            [sexp_free].
 *   [line] Line on which this symbol appeared.
 *   [column] Column on the line where this symbol appeared.
 */
struct Sexp* sexp_symbol(const char* string, u_int32_t line, u_int32_t column) {
    struct Symbol* symbol = malloc(sizeof(struct Symbol));
    symbol->str = string;
    symbol->line = line;
    symbol->col = column;
    struct Sexp* sexp = malloc(sizeof(struct Sexp));
    sexp->type = SYM;
    sexp->val.sym = symbol;
    return sexp;
}

void sexp_free_list(struct Sexp* sexp) {
    // Invariant: sexp always of type LIST or NIL.
    while (sexp->type != NIL) {
        struct Sexp* car = sexp->val.list.car;
        sexp_free(car);
        struct Sexp* cdr = sexp->val.list.cdr;
        free(sexp);
        sexp = cdr;
    }
    // Otherwise, it is NIL, stop freeing the list.
    // There is only one instance of Nil which is not ever freed.
}

void sexp_free(struct Sexp* sexp) {
    if (sexp->type == SYM) {
        struct Symbol* sym = sexp->val.sym;
        free((char*)sym->str);
        free(sym);
    } else if (sexp->type == LIST) {
        sexp_free_list(sexp);
    } // Otherwise, it is NIL, do nothing.

    // There is only one instance of Nil which is not ever freed.
    if (sexp->type != NIL) {
        free(sexp);
    }
}
