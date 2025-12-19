#pragma once

struct String {
    unsigned int length;
    unsigned int size;
    char* mem;
};

struct String* string_alloc(unsigned int size);
void string_free(struct String* string);

struct String* string_from_bytes(const char* mem, unsigned int size);
struct String* string_from_cstr(const char* cstr);

void string_add(struct String* s1, struct String* s2);
void string_add_char(struct String* s, char c);
void string_add_bytes(struct String* s, const char* mem, unsigned int size);
void string_add_cstr(struct String* string, const char* str);

int string_cmp(struct String* s1, struct String* s2);
int string_cmp_cstr(struct String* s, const char* cstr);

bool string_eq(struct String* s1, struct String* s2);
bool string_eq_cstr(struct String* s, const char* cstr);
