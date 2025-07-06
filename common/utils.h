#ifndef __COMMON_UTILS_H
#define __COMMON_UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "vector.h"

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define ARRAY_LEN(arr) (sizeof(arr)/sizeof(arr[0]))

#define UNUSED(x) (void)(x)
#define UNREACHABLE(msg) \
    do { \
        fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, msg); \
        exit(EXIT_FAILURE); \
    } while(0)

#define vector_push_word_back(vec, w) \
    do { \
        vector_push_back(vec, w >> 8); \
        vector_push_back(vec, w & 0xFF); \
    } while(0)

bool string_in_args(const char *str, size_t count, ...);
bool string_in_array(const char *str, const char *array[], size_t array_len);

#endif
