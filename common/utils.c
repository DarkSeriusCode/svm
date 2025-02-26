#include "utils.h"
#include <stdarg.h>
#include <string.h>

bool string_in_args(const char *str, size_t count, ...) {
    va_list args;
    va_start(args, count);
    for (size_t i = 0; i < count; i++) {
        if (strcmp(str, va_arg(args, char*)) == 0) {
            va_end(args);
            return true;
        }
    }
    va_end(args);
    return false;
}
