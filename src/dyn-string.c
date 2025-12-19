#include <stdlib.h>
#include <string.h>

#include "../inc/dyn-string.h"

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

void string_free(struct String* s) {
    free(s->mem);
    free(s);
}

struct String* string_from_bytes(const char* mem, unsigned int size) {
    struct String* s = string_alloc(size);
    memcpy(s->mem, mem, size);
    s->length = size;
    return s;
}

struct String* string_from_cstr(const char* cstr) {
    unsigned int len = strlen(cstr);
    return string_from_bytes(cstr, len);
}

void string_add(struct String* s1, struct String* s2) {
    while (s1->size - s1->length < s2->length) {
        s1->mem = realloc(s1->mem, s1->size * 2);
        s1->size *= 2;
    }
    memcpy(s1->mem + s1->length, s2->mem, s2->length);
    s1->length = s1->length + s2->length;
}

void string_add_char(struct String* s, char c) {
    if (s->size - s->length == 0) {
        s->mem = realloc(s->mem, s->size * 2);
        s->size *= 2;
    }
    s->mem[s->length] = c;
    s->length++;
}

void string_add_bytes(struct String* s, const char* mem, unsigned int size) {
    while (s->size - s->length < size) {
        s->mem = realloc(s->mem, s->size * 2);
        s->size *= 2;
    }
    memcpy(s->mem + s->length, mem, size);
    s->length = s->length + size;
}

void string_add_cstr(struct String* s, const char* cstr) {
    unsigned int len = strlen(cstr);
    string_add_bytes(s, cstr, len);
}
