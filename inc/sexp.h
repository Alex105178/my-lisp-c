#pragma once

#include <sys/types.h>

#include "../inc/dyn-string.h"

/* struct Symbol
 * Represents a single word/identifier. Can contain any characters other than
 * '(', ')', ' ', '\n', '\r', or '\0'.
 */
struct Symbol {
    const char *str;
    u_int32_t line;
    u_int32_t col;
};

/* struct Sexp
 * A sum-type for symbols and s-expression lists. [type] encodes what type of
 * value is stored in [val].
 */
struct Sexp {
    enum {NIL, SYM, LIST} type;
    union {
        struct Symbol* sym;
        struct {
            struct Sexp* car;
            struct Sexp* cdr;
        } list;
    } val;;
};

extern struct Sexp Nil;

void string_sexp(struct Sexp* sexp, struct String* string);
struct Sexp* sexp_cons(struct Sexp *car, struct Sexp *cdr);
struct Sexp* sexp_symbol(const char* string, u_int32_t line, u_int32_t column);
