#ifndef __COMMON_UTILS_H
#define __COMMON_UTILS_H

#include <stdbool.h>
#include <stddef.h>

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

bool string_in_args(const char *str, size_t count, ...);

#endif
