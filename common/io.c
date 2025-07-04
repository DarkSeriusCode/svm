#include "io.h"
#include <stdlib.h>

char *read_whole_file(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        error_file_doesnot_exist(filename);
    }
    // getting len
    fseek(fp, 0, SEEK_END);
    size_t file_len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buffer = malloc(file_len + 1);
    fread(buffer, 1, file_len + 1, fp);
    buffer[file_len] = '\0';

    fclose(fp);
    return buffer;
}

size_t load_program(byte *memory, size_t memory_size, const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        error_file_doesnot_exist(filename);
    }
    // getting len
    fseek(fp, 0, SEEK_END);
    size_t file_len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    fread(memory, memory_size, sizeof(byte), fp);
    fclose(fp);
    return file_len;
}

void error_invalid_file_format(const char *filename) {
    style(STYLE_BOLD);
    printf("%s: ", filename);
    printf_red("invalid file format\n");
    exit(EXIT_FAILURE);
}

void error_file_doesnot_exist(const char *filename) {
    printf("File not found: %s\n", filename);
    exit(EXIT_FAILURE);
}

void error_couldnot_find_section(const char *section_name) {
    style(STYLE_BOLD);
    printf_red("cannot find section");
    style(STYLE_BOLD);
    printf(": \"");
    printf_green("%s", section_name);
    style(STYLE_BOLD);
    printf("\"\n");
    exit(EXIT_FAILURE);
}
