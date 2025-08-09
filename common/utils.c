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

bool string_in_array(const char *str, const char *array[], size_t array_len) {
    for (size_t i = 0; i < array_len; i++)
        if (strcmp(array[i], str) == 0) return true;
    return false;
}

bool instropcode_in_args(InstrOpcode opcode, size_t count, ...) {
    va_list args;
    va_start(args, count);
    for (size_t i = 0; i < count; i++) {
        if (opcode == va_arg(args, InstrOpcode)) {
            va_end(args);
            return true;
        }
    }
    va_end(args);
    return false;
}

bool instropcode_in_array(InstrOpcode opcode, const InstrOpcode *array, size_t array_len) {
    for (size_t i = 0; i < array_len; i++)
        if (array[i] == opcode) return true;
    return false;
}

bool diropcode_in_array(DirOpcode opcode, const DirOpcode *array, size_t array_len) {
    for (size_t i = 0; i < array_len; i++)
        if (array[i] == opcode) return true;
    return false;
}
