#ifndef __COMMON_UTILS_H
#define __COMMON_UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

#define UNUSED(x) (void)(x)
#define UNREACHABLE(msg) \
    do { \
        fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, msg); \
        exit(EXIT_FAILURE); \
    } while(0)

bool string_in_args(const char *str, size_t count, ...);

#endif
