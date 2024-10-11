#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "io.h"

char *read_whole_file(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        exit_with_erorr("Cannot open a file %s\n", filename);
    }
    // getting len
    fseek(fp, 0, SEEK_END);
    size_t file_len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buffer = malloc(file_len + 1);
    fread(buffer, 1, file_len, fp);
    buffer[file_len] = '\0';

    fclose(fp);
    return buffer;
}

void exit_with_erorr(const char *error_msg, ...) {
    va_list va;
    va_start(va, error_msg);
    fprintf(stderr, error_msg, va);
    exit(EXIT_FAILURE);
    va_end(va);
}
