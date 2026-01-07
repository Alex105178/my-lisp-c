#pragma once

#include <stdbool.h>

enum ValueType {
    VT_ERROR,
    VT_SEXP,
    VT_INTEGER,
    VT_STRING,
    VT_BOOLEAN,
    VT_LAMBDA
};

struct Error {
    const char* msg;
};

struct Lambda;

struct Value {
    enum ValueType vt;
    union {
        struct Error error;
        struct Sexp* sexp;
        struct String* string;
        long integer;
        bool boolean;
        struct Lambda* lambda;
    } val;
    int ref_count;
};

struct Binding {
    struct Symbol* id;
    struct Value* val;
    struct Binding* next; // May be NULL
    unsigned int ref_count;
};

struct Lambda {
    struct Sexp* body;
    struct Binding* context;
    struct Symbol* arg;
};

extern struct Value NilV;

struct String* value_to_string(struct Value* v);
struct Value* eval(struct Sexp* sexp, struct Binding* bindings);
void value_free(struct Value* val);
