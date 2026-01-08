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
        struct String* s = sexp_to_string(v->val.sexp);
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
    case VT_LAMBDA: {
        struct String* s = string_from_cstr("(lambda (");
        string_add(s, v->val.lambda->arg->str);
        string_add_cstr(s, ") ");
        struct String* body = sexp_to_string(v->val.lambda->body);
        string_add(s, body);
        string_free(body);
        string_add_char(s, ')');
        return s;
    }
    }
}

void dec_ref_all_bindings(struct Binding* bindings);

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
    case VT_LAMBDA:
        dec_ref_all_bindings(val->val.lambda->context);
        free(val->val.lambda);
        free(val);
    }
}

struct Value* value_make_lambda(struct Sexp* body, struct Binding* context,
                                struct Symbol* arg) {
    struct Lambda* lambda = malloc(sizeof(struct Lambda));
    lambda->body = body;
    lambda->context = context;
    lambda->arg = arg;
    struct Value* value = malloc(sizeof(struct Value));
    value->vt = VT_LAMBDA;
    value->val.lambda = lambda;
    value->ref_count = 1;
    return value;
}

void value_inc_ref(struct Value* val) {
    val->ref_count++;
}

void value_dec_ref_or_free(struct Value* val) {
    // If the ref count is -1, that represents an object that cannot be freed.
    if (-1 != val->ref_count) {
        if (1 < val->ref_count) {
            val->ref_count--;
        } else {
            value_free(val);
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
    val->ref_count = 1;
    return val;
};

struct Value* make_integer(long l) {
    struct Value* val = malloc(sizeof(struct Value));
    val->vt = VT_INTEGER;
    val->val.integer = l;
    val->ref_count = 1;
    return val;
}

struct Value* make_boolean(bool b) {
    struct Value* val = malloc(sizeof(struct Value));
    val->vt = VT_BOOLEAN;
    val->val.boolean = b;
    val->ref_count = 1;
    return val;
}

struct Value NilV = {VT_SEXP, .val.sexp = &Nil, .ref_count = -1};

struct Binding* add_binding(struct Symbol* id, struct Value* val,
                            struct Binding* next) {
    struct Binding* binding = malloc(sizeof(struct Binding));
    binding->id = id;
    binding->val = val;
    binding->next = next;
    binding->ref_count = 1;
    return binding;
}

void inc_ref_all_bindings(struct Binding* bindings) {
    while (bindings != NULL) {
        struct Binding* next = bindings->next;
        bindings->ref_count++;
        bindings = next;
    }
}

void dec_ref_all_bindings(struct Binding* bindings) {
    while (bindings != NULL) {
        struct Binding* next = bindings->next;
        if (1 < bindings->ref_count) {
            bindings->ref_count--;
        } else {
            value_dec_ref_or_free(bindings->val);
            free(bindings);
        }
        bindings = next;
    }
}

struct Binding* dec_ref_top_binding(struct Binding* bindings) {
    if (NULL == bindings) {
        // TODO: Should this cause an error instead?
        return bindings;
    }
    // bindings->id; Don't free. Belongs to the s-expr being evaluated.
    struct Binding* next = bindings->next;
    if (1 < bindings->ref_count) {
        bindings->ref_count--;
    } else {
        value_dec_ref_or_free(bindings->val);
        free(bindings);
    }
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

struct Value* eval_boolean(struct Sexp* sexp) {
    if (sexp->type != SYM) {
        return make_error("Impossible!");
    }
    if (string_eq_cstr(sexp->val.sym->str, "true")) {
        return make_boolean(true);
    } else if (string_eq_cstr(sexp->val.sym->str, "false")) {
        return make_boolean(false);
    } else {
        return make_error("Could not parse boolean!");
    }
}

struct Value* eval(struct Sexp* sexp, struct Binding* bindings);

enum INT_OP {INT_ADD, INT_SUB, INT_MUL, INT_DIV, INT_EQ};

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
                        struct Value* res;
                        switch (op) {
                        case INT_ADD:
                            res = make_integer(int_a1 + int_a2);
                            break;
                        case INT_SUB:
                            res = make_integer(int_a1 - int_a2);
                            break;
                        case INT_MUL:
                            res = make_integer(int_a1 * int_a2);
                            break;
                        case INT_DIV:
                            if (0 == int_a2) {
                                value_dec_ref_or_free(arg1);
                                value_dec_ref_or_free(arg2);
                                return make_error("Cannot divide by zero!");
                            }
                            res = make_integer(int_a1 / int_a2);
                            break;
                        case INT_EQ:
                            res = make_boolean(int_a1 == int_a2);
                            break;
                        }
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

struct Value* eval_if(struct Sexp* sexp, struct Binding* bindings) {
    struct Sexp* cdr = sexp->val.list.cdr;
    if (cdr->type != LIST) {
        return make_error("Expected a conditional guard!");
    }
    struct Sexp* cond = cdr->val.list.car;
    struct Sexp* cddr = cdr->val.list.cdr;
    if (cddr->type != LIST) {
        return make_error("Expected a true branch!");
    }
    struct Sexp* true_branch = cddr->val.list.car;
    struct Sexp* cdddr = cddr->val.list.cdr;
    if (cdddr->type != LIST) {
        return make_error("Expected a false branch!");
    }
    struct Sexp* false_branch = cdddr->val.list.car;
    struct Sexp* cddddr = cdddr->val.list.cdr;
    if (cddddr->type != NIL) {
        return make_error("Unexpected extra form after the false branch!");
    }

    struct Value* cond_val = eval(cond, bindings);

    if (cond_val->vt == VT_ERROR) {
        return cond_val;
    }

    struct Value* res;
    if (cond_val->vt == VT_BOOLEAN && cond_val->val.boolean == false) {
        res = eval(false_branch, bindings);
    } else {
        res = eval(true_branch, bindings);
    }
    value_dec_ref_or_free(cond_val);
    return res;
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

struct Value* eval_lambda(struct Sexp* sexp, struct Binding* bindings) {
    struct Sexp* cdr = sexp->val.list.cdr;
    if (cdr->type != LIST) {
        return make_error("Expected an argument and body!");
    }
    struct Sexp* cadr = cdr->val.list.car;
    if (cadr->type != LIST) {
        return make_error("Expected argument list!");
    }
    struct Sexp* arg = cadr->val.list.car;
    if (arg->type != SYM) {
        return make_error("Argument form must be a symbol!");
    }
    struct Sexp* cdadr = cadr->val.list.cdr;
    if (cdadr->type != NIL) {
        return make_error("More than one argument!");
    }
    struct Sexp* cddr = cdr->val.list.cdr;
    if (cddr->type != LIST) {
        return make_error("Expected a body!");
    }
    struct Sexp* body = cddr->val.list.car;
    struct Sexp* cdddr = cddr->val.list.cdr;
    if (cdddr->type != NIL) {
        return make_error("Too many forms!");
    }

    inc_ref_all_bindings(bindings);

    return value_make_lambda(body, bindings, arg->val.sym);
}

struct Value* eval_apply(struct Sexp* sexp, struct Binding* bindings) {
    struct Sexp* car = sexp->val.list.car;

    struct Value* lambda = eval(car, bindings);
    if (lambda->vt != VT_LAMBDA) {
        if (lambda->vt == VT_ERROR) {
            return lambda;
        } else {
            value_dec_ref_or_free(lambda);
            return make_error(
                "Expected a lambda value in function application!");
        }
    }

    struct Sexp* cdr = sexp->val.list.cdr;
    if (cdr->type != LIST) {
        value_dec_ref_or_free(lambda);
        return make_error("Expected an argument!");
    }
    struct Sexp* arg = cdr->val.list.car;
    struct Sexp* cddr = cdr->val.list.cdr;
    if (cddr->type != NIL) {
        value_dec_ref_or_free(lambda);
        return make_error("Expected only one argument!");
    }
    struct Value* arg_val = eval(arg, bindings);
    struct Binding* ctx = add_binding(lambda->val.lambda->arg, arg_val,
                                      lambda->val.lambda->context);
    struct Value* res = eval(lambda->val.lambda->body, ctx);
    dec_ref_top_binding(ctx);
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
            if (v2->vt != VT_ERROR) {
                return v2;
            } else {
                value_free(v2);
                struct Value* v3 = eval_boolean(sexp);
                if (v3->vt != VT_ERROR) {
                    return v3;
                }
                value_free(v3);
                return make_error("Unrecognized identifier!");
            }
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
            } else if (string_eq_cstr(car->val.sym->str, "=")) {
                return eval_intop(sexp, INT_EQ, bindings);
            } else if (string_eq_cstr(car->val.sym->str, "if")) {
                return eval_if(sexp, bindings);
            } else if (string_eq_cstr(car->val.sym->str, "let")) {
                return eval_let(sexp, bindings);
            } else if (string_eq_cstr(car->val.sym->str, "lambda")) {
                return eval_lambda(sexp, bindings);
            } else {
                return eval_apply(sexp, bindings);
            }
        } else { // car->type == LIST
            return eval_apply(sexp, bindings);
        }
    } else { // sexp->type == NIL
        return &NilV;
    }
};
