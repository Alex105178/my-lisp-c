#pragma once

struct String {
    unsigned int length;
    unsigned int size;
    char* mem;
};

struct String* string_alloc(unsigned int size);
void string_free(struct String* string);
void string_add(struct String* string, const char* str);
