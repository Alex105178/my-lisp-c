#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define NDEBUG

/* struct Symbol
 * Represents a single word/identifier. Can contain any characters other than
 * '(', ')', ' ', '\n', '\r', or '\0'.
 */
struct Symbol {
    char* string;
};

/* struct Sexp
 * A sum-type for symbols and s-expression lists. [type] encodes what type of
 * value is stored in [val].
 */
struct Sexp {
    enum {NIL, SYM, LIST} type;
    union {
        struct Symbol sym;
        struct {
            struct Sexp* car;
            struct Sexp* cdr;
        } list;
    } val;;
};

struct String {
    unsigned int length;
    unsigned int size;
    char* mem;
};

struct String* string_alloc(unsigned int size) {
    if (size == 0) {
        size = 1;
    }
    struct String* string = malloc(sizeof(struct String));
    string->length = 0;
    string->size = size;
    string->mem = malloc(size);
    return string;
}

void string_add(struct String* string, const char* str) {
    int str_len = strlen(str);
    int str_sz = str_len + 1;
    while (string->size - string->length < str_sz) {
        string->mem = realloc(string->mem, string->size * 2);
        string->size = string->size * 2;
    }
    memcpy(string->mem + string->length, str, str_sz);
    string->length = string->length + str_len;
}

struct Sexp Nil = {NIL, .val.list = {(void*)5, (void*)5}};

void string_sexp(struct Sexp *sexp, struct String *string) {
    if (sexp->type == SYM) {
        string_add(string, sexp->val.sym.string);
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
                string_add(string, cur->val.sym.string);
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

struct Sexp* sexp_symbol(struct Symbol symbol) {
    struct Sexp* sexp = malloc(sizeof(struct Sexp));
    sexp->type = SYM;
    sexp->val.sym = symbol;
    return sexp;
}

enum ParseError {
    PE_NONE,
    PE_UNBAL_R_PAREN,
    PE_UNBAL_L_PAREN,
    PE_INVALID_DOT_LOC,
    PE_INVALID_DOT_NOTATION,
};

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
    struct Sexp* sexp = sexp_symbol((struct Symbol){sym_str});
    struct ParseRes res = {PE_NONE, {sexp, next}};
    return res;
}

struct ParseRes parse_sexp(const char* str);

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
