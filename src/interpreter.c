#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../inc/dyn-string.h"
#include "../inc/sexp.h"

#include "../inc/interpreter.h"

struct String* value_to_string(struct Value* v) {
    switch (v->vt) {
    case VT_ERROR:
        return string_from_cstr(v->val.error.msg);
        break;
    case VT_SEXP: {
        struct String* s = string_alloc(8);
        string_sexp(v->val.sexp, s);
        return s;
    }
        break;
    case VT_INTEGER:
        return long_to_string(v->val.integer);
        break;
    case VT_STRING:
        return v->val.string;
    case VT_BOOLEAN:
        return v->val.boolean ?
            string_from_cstr("true") :
            string_from_cstr("false");
    }
}

void value_free(struct Value* val) {
    switch (val->vt) {
    case VT_INTEGER:
    case VT_BOOLEAN:
        free(val);
        break;
    case VT_ERROR:
        free((char*)val->val.error.msg);
        free(val);
        break;
    case VT_STRING:
        string_free(val->val.string);
        free(val);
    case VT_SEXP:
        sexp_free(val->val.sexp);
        free(val);
        break;
    }
}    

void value_inc_ref(struct Value* val) {
    val->ref_count++;
}

void value_dec_ref_or_free(struct Value* val) {
    if (-1 != val->ref_count) {
        if (0 == val->ref_count) {
            value_free(val);
        } else {
            val->ref_count--;
        }
    }    
}

struct Value* make_error(const char* msg) {
    struct Value* val = malloc(sizeof(struct Value));
    int msg_size = strlen(msg) + 1;
    char* str = malloc(msg_size);
    memcpy(str, msg, msg_size);
    val->vt = VT_ERROR;
    val->val.error = (struct Error){str};
    val->ref_count = 0;
    return val;
};

struct Value* make_integer(long l) {
    struct Value* val = malloc(sizeof(struct Value));
    val->vt = VT_INTEGER;
    val->val.integer = l;
    val->ref_count = 0;
    return val;
}

struct Value NilV = {VT_SEXP, .val.sexp = &Nil, .ref_count = -1};

struct Binding* add_binding(struct Symbol* id, struct Value* val,
                            struct Binding* next) {
    struct Binding* binding = malloc(sizeof(struct Binding));
    binding->id = id;
    binding->val = val;
    binding->next = next;
    return binding;
}

struct Binding* dec_ref_top_binding(struct Binding* bindings) {
    if (NULL == bindings) {
        // TODO: Should this cause an error instead?
        return bindings;
    }
    // bindings->id; Don't free. Belongs to the s-expr being evaluated.
    value_dec_ref_or_free(bindings->val);
    struct Binding* next = bindings->next;
    free(bindings);
    return next;
}

struct Value* find_binding(struct Binding* binds, struct Symbol* sym) {
    if (binds == NULL) {
        return make_error("Undefined Variable!");
    }
    struct Binding* bind = binds;
    while (true) {
        if (string_eq(bind->id->str, sym->str)) {
            value_inc_ref(bind->val);
            return bind->val;
        } else {
            if (NULL != bind->next) {
                bind = bind->next;
            } else {
                return make_error("Undefined Variable!");
            }
        }
    }
};

struct Value* eval_integer(struct Sexp* sexp) {
    if (sexp->type != SYM) {
        return make_error("Impossible!");
    }
    struct string_long_res res = string_to_long(sexp->val.sym->str);
    if (res.good) {
        return make_integer(res.res);
    } else {
        return make_error("Could not parse integer!");
    }
}

struct Value* eval(struct Sexp* sexp, struct Binding* bindings);

enum INT_OP {INT_ADD, INT_SUB, INT_MUL, INT_DIV};

struct Value* eval_intop(struct Sexp* sexp, enum INT_OP op,
                         struct Binding* bindings) {
    struct Sexp* cdr = sexp->val.list.cdr;
    if (cdr->type != LIST) {
        return make_error("Expected a term!");
    } else {
        struct Sexp* car2 = cdr->val.list.car;
        struct Sexp* cdr2 = cdr->val.list.cdr;
        if (car2->type == NIL) {
            return make_error("Expected a term!");
        } else {
            struct Value* arg1 = eval(car2, bindings);
            if (arg1->vt == VT_ERROR) {
                return arg1;
            }
            if (cdr2->type != LIST) {
                value_dec_ref_or_free(arg1);
                return make_error("Expected a term!");
            } else {
                struct Sexp* car3 = cdr2->val.list.car;
                struct Sexp* cdr3 = cdr->val.list.cdr;
                if (car3->type == NIL) {
                    value_dec_ref_or_free(arg1);
                    return make_error("Expected a term!");
                } else {
                    struct Value* arg2 = eval(car3, bindings);
                    if (arg2->vt == VT_ERROR) {
                        value_dec_ref_or_free(arg1);
                        return arg2;
                    }
                    if (arg1->vt == VT_INTEGER && arg2->vt == VT_INTEGER) {
                        long int_a1 = arg1->val.integer;
                        long int_a2 = arg2->val.integer;
                        long long_res;
                        switch (op) {
                        case INT_ADD:
                            long_res = int_a1 + int_a2;
                            break;
                        case INT_SUB:
                            long_res = int_a1 - int_a2;
                            break;
                        case INT_MUL:
                            long_res = int_a1 * int_a2;
                            break;
                        case INT_DIV:
                            if (0 == int_a2) {
                                value_dec_ref_or_free(arg1);
                                value_dec_ref_or_free(arg2);
                                return make_error("Cannot divide by zero!");
                            }
                            long_res = int_a1 / int_a2;
                            break;
                        }
                        struct Value* res = make_integer(long_res);
                        value_dec_ref_or_free(arg1);
                        value_dec_ref_or_free(arg2);
                        return res;
                    } else {
                        value_dec_ref_or_free(arg1);
                        value_dec_ref_or_free(arg2);
                        return make_error("Invalid argument!");
                    }
                }
            }
        }
    }
}

struct Value* eval_let(struct Sexp* sexp, struct Binding* bindings) {
    struct Sexp* cdr = sexp->val.list.cdr;
    struct Sexp* cadr = cdr->val.list.car;
    struct Sexp* cddr = cdr->val.list.cdr;
    struct Sexp* id = cadr->val.list.car;
    struct Sexp* cdadr = cadr->val.list.cdr;
    struct Sexp* val = cdadr->val.list.car;
    struct Sexp* cddadr = cdadr->val.list.cdr;
    struct Sexp* body = cddr->val.list.car;
    struct Sexp* cdddr = cddr->val.list.cdr;

    bool invalid_form =
        cdr->type != LIST ||
        cadr->type != LIST || cddr->type != LIST ||
        id->type != SYM || cdadr->type != LIST ||
        val->type == NIL || cddadr->type != NIL ||
        body->type == NIL || cdddr->type != NIL;

    if (invalid_form) {
        return make_error("Invalid let form!");
    }

    struct Value* value = eval(val, bindings);
    struct Binding* new_bindings = add_binding(id->val.sym, value, bindings);
    value = NULL;
    struct Value* res = eval(body, new_bindings);
    dec_ref_top_binding(new_bindings);
    return res;
}

struct Value* eval(struct Sexp* sexp, struct Binding* bindings) {
    if (sexp->type == SYM) {
        struct Value* v = find_binding(bindings, sexp->val.sym);
        if (v->vt != VT_ERROR) {
            return v;
        } else {
            value_free(v);
            struct Value* v2 = eval_integer(sexp);
            return v2;
        }
    } else if (sexp->type == LIST) {
        struct Sexp* car = sexp->val.list.car;
        if (car->type == NIL) {
            return make_error("Empty S-expression list!");
        } else if (car->type == SYM) {
            if (string_eq_cstr(car->val.sym->str, "+")) {
                return eval_intop(sexp, INT_ADD, bindings);
            } else if (string_eq_cstr(car->val.sym->str, "-")) {
                return eval_intop(sexp, INT_SUB, bindings);
            } else if (string_eq_cstr(car->val.sym->str, "*")) {
                return eval_intop(sexp, INT_MUL, bindings);
            } else if (string_eq_cstr(car->val.sym->str, "/")) {
                return eval_intop(sexp, INT_DIV, bindings);
            } else if (string_eq_cstr(car->val.sym->str, "let")) {
                return eval_let(sexp, bindings);
            } else {
                return make_error("Invalid form!");
            }
        } else { // car->type == LIST
            return eval(car, bindings);
        }
    } else { // sexp->type == NIL
        return &NilV;
    }
};
