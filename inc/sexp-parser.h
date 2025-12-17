#pragma once

#include "sexp.h"

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

const char *parse_error_to_str(enum ParseError pe);
struct NextToken peek_next_token(const char *str);
struct ParseRes parse_symbol(const char* str);
struct ParseRes parse_sexp_list(const char *str);
struct ParseRes parse_sexp(const char* str);
