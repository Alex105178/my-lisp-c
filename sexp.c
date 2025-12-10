#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "sexp.h"
#include "dyn-string.h"

// #define NDEBUG

struct Sexp Nil = {NIL, .val.list = {(void*)5, (void*)5}};

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

const char *parse_error_to_str(enum ParseError pe) {
    unsigned int len;
    char* str;
    char* mem;
    switch (pe) {
    case PE_NONE:
        str = "None";
        len = strlen(str) + 1;
        mem = malloc(len);
        memcpy(mem, str, len);
        return mem;
    case PE_UNBAL_R_PAREN:
        str = "Unbalanced Right Parenthesis";
        len = strlen(str) + 1;
        mem = malloc(len);
        memcpy(mem, str, len);
        return mem;
    case PE_UNBAL_L_PAREN:
        str = "Unbalanced Left Parenthesis";
        len = strlen(str) + 1;
        mem = malloc(len);
        memcpy(mem, str, len);
        return mem;
    case PE_INVALID_DOT_LOC:
        str = "Invalid Dot '.' notation";
        len = strlen(str) + 1;
        mem = malloc(len);
        memcpy(mem, str, len);
        return mem;
    }
    str = "No description";
    len = strlen(str) + 1;
    mem = malloc(len);
    memcpy(mem, str, len);
    return mem;
}

struct NextToken peek_next_token(const char *str) {
    const char *next = str;
    while (true) {
        switch (*next) {
        case ' ':
        case '\n':
        case '\r':
            next++;
            break;
        case '.':
            return (struct NextToken){T_DOT, next};
        case '(':
            return (struct NextToken){T_L_PAREN, next};
        case ')':
            return (struct NextToken){T_R_PAREN, next};
        case '\0':
            return (struct NextToken){T_EOF, next};
        default:
            return (struct NextToken){T_SYMBOL, next};
        }
    }
}

struct ParseRes parse_symbol(const char* str) {
    const char* start = str;
    const char* next = str;
    int cont = true;
    while (cont) {
        switch (*next) {
        case ' ':
        case '(':
        case ')':
        case '\n':
        case '\r':
        case '\0':
            cont = false;
            break;
        default:
            next++;
        }
    }
    assert(start != next);
    int num_symbol_chars = next - start;
    char* sym_str = malloc(sizeof(char) * (num_symbol_chars + 1));
    memcpy(sym_str, start, num_symbol_chars);
    sym_str[num_symbol_chars] = '\0';
    // todo: Update to use line and col values.
    struct Sexp* sexp = sexp_symbol(sym_str, 0, 0);
    struct ParseRes res = {PE_NONE, {sexp, next}};
    return res;
}

struct ParseRes parse_sexp_list(const char *str) {
    const char* next = str;
    struct NextToken tok = peek_next_token(next);
    next = tok.next_char;
    if (tok.kind == T_R_PAREN) {
        return (struct ParseRes){PE_NONE, {&Nil, next + 1}};
    }
    struct ParseRes res = parse_sexp(next);
    if (res.error != PE_NONE) {
        return res;
    }
    struct Sexp* root = sexp_cons(res.val.good.sexp, &Nil);
    struct Sexp* cur = root;
    next = res.val.good.next_char;
    while (true) {
        struct NextToken tok = peek_next_token(next);
        next = tok.next_char;
        if (tok.kind == T_R_PAREN) {
            return (struct ParseRes){PE_NONE, {root, next + 1}};
        }
        if (tok.kind == T_DOT) {
            struct NextToken tok = peek_next_token(next + 1);
            next = tok.next_char;
            if (tok.kind != T_R_PAREN) {
                struct ParseRes res = parse_sexp(next);
                if (res.error != PE_NONE) {
                    return res;
                }
                next = res.val.good.next_char;
                struct NextToken tok = peek_next_token(next);
                if (tok.kind == T_R_PAREN) {
                    cur->val.list.cdr = res.val.good.sexp;
                    return (struct ParseRes){PE_NONE, {root, next + 1}};
                } else {
                    return (struct ParseRes){PE_INVALID_DOT_NOTATION, {.bad = tok.next_char}};
                }
            } else {
                assert(false);
            }
        }
        struct ParseRes res = parse_sexp(next);
        if (res.error != PE_NONE) {
            return res;
        }
        struct Sexp* new = sexp_cons(res.val.good.sexp, &Nil);
        cur->val.list.cdr = new;
        cur = new;
        next = res.val.good.next_char;
    }
    
}

struct ParseRes parse_sexp(const char* str) {
    struct NextToken tok = peek_next_token(str);
    const char *next = tok.next_char;
    switch (tok.kind) {
    case T_L_PAREN:
        return parse_sexp_list(next + 1);
    case T_R_PAREN:
        return (struct ParseRes){PE_UNBAL_R_PAREN, {.bad = next}};
    case T_SYMBOL:
        return parse_symbol(next);
    case T_DOT:
        return (struct ParseRes){PE_INVALID_DOT_LOC, {.bad = next}};
    case T_EOF:
        return (struct ParseRes){PE_UNBAL_L_PAREN, {.bad = next}};
    }
}
