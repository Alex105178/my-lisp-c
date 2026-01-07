#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../inc/dyn-string.h"
#include "../inc/test-generic.h"

void run_tests(char* list[][3], int len, struct String* (*test)(char* input)) {
    int num_success = 0;
    int num_fail = 0;
    for (int i = 0; i < len; i = i + 1) {
        char** entry = list[i];
        if (0 == strcmp(entry[0], "==")) {
            struct String* result = test(entry[1]);
            if (0 == strcmp(result->mem, entry[2])) {
                num_success += 1;
                free(result->mem);
            } else {
                printf("Test %d failed:\n"
                       "  \"%s\" -x-> \"%s\"\n"
                       "  \"%s\" ---> \"%s\"\n",
                       i, entry[1], entry[2], entry[1], result->mem);
                num_fail += 1;
                free(result->mem);
                free(result);
            }
        } else {
            fprintf(stderr, "Error, invalid compare operator!");
            assert(false);
        }
    }
    printf("Success: %d, Fail: %d, Total: %d\n", num_success, num_fail, len);
}
