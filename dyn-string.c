#include <stdlib.h>
#include <string.h>

#include "dyn-string.h"

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
