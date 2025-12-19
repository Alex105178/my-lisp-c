#include <stdbool.h>
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

int string_cmp(struct String* s1, struct String* s2) {
    unsigned int min_len = s1->length < s2->length ? s1->length : s2->length;
    int res = strncmp(s1->mem, s2->mem, min_len);
    if (res == 0) {
        if (s1->length < s2->length) {
            return -1 * s2->mem[min_len];
        } else if (s1->length > s2->length) {
            return s1->mem[min_len];
        } else {
            return 0;
        }
    } else {
        return res;
    }
}

int string_cmp_cstr(struct String* s, const char* cstr) {
    unsigned int cstr_len = strlen(cstr);
    unsigned int min_len = s->length < cstr_len ? s->length : cstr_len;
    int res = strncmp(s->mem, cstr, min_len);
    if (res == 0) {
        if (s->length < cstr_len) {
            return -1 * cstr[min_len];
        } else if (s->length > cstr_len) {
            return s->mem[min_len];
        } else {
            return 0;
        }
    } else {
        return res;
    }
}

bool string_eq(struct String* s1, struct String* s2) {
    if (s1->length != s2->length) {
        return false;
    } else {
        return 0 == strncmp(s1->mem, s2->mem, s1->length);
    }
}

bool string_eq_cstr(struct String* s, const char* cstr) {
    unsigned int cstr_len = strlen(cstr);
    if (s->length != cstr_len) {
        return false;
    } else {
        return 0 == strncmp(s->mem, cstr, s->length);
    }
}
