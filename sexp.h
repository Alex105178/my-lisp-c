#pragma once

#include <sys/types.h>

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

enum ParseError {
    PE_NONE,
    PE_UNBAL_R_PAREN,
    PE_UNBAL_L_PAREN,
    PE_INVALID_DOT_LOC,
    PE_INVALID_DOT_NOTATION,
};

struct ParseRes {
    enum ParseError error;
    union {
        struct {
            struct Sexp* sexp;
            const char* next_char;
        } good;
        struct {
            const char* loc;
        } bad;
    } val;
};

struct NextToken {
    enum { T_SYMBOL, T_L_PAREN, T_R_PAREN, T_EOF, T_DOT } kind;
    const char* next_char;
};

struct String* string_alloc(unsigned int size);
void string_add(struct String* string, const char* str);
void string_sexp(struct Sexp *sexp, struct String *string);
const char *parse_error_to_str(enum ParseError pe);
struct NextToken peek_next_token(const char *str);
struct ParseRes parse_symbol(const char* str);
struct ParseRes parse_sexp_list(const char *str);
struct ParseRes parse_sexp(const char* str);
