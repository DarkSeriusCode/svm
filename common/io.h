#ifndef __COMMON_IO_H
#define __COMMON_IO_H

#include <stdio.h>
#include "arch.h"

extern bool ENABLE_COLORS;
typedef const char * Color;

#define COLOR_RED "\033[31m"
#define COLOR_CYAN "\033[36m"
#define COLOR_GREEN "\033[32m"
#define COLOR_BLUE "\033[34m"
#define COLOR_YELLOW "\033[33m"
#define STYLE_BOLD "\033[1m"
#define STYLE_UNDERLINED "\033[4m"
#define STYLE_NONE "\033[0m"

#define printf_with_color(color, ...) \
    do { \
        if (ENABLE_COLORS && color) printf("%s", color); \
        printf(__VA_ARGS__); \
        printf(STYLE_NONE); \
    } while(0);

#define printf_red(...) printf_with_color(COLOR_RED, __VA_ARGS__)
#define printf_cyan(...) printf_with_color(COLOR_CYAN, __VA_ARGS__)
#define printf_green(...) printf_with_color(COLOR_GREEN, __VA_ARGS__)
#define printf_blue(...) printf_with_color(COLOR_BLUE, __VA_ARGS__)
#define printf_yellow(...) printf_with_color(COLOR_YELLOW, __VA_ARGS__)
#define style(style) if (ENABLE_COLORS) printf(style)

// Returns file content. Needs to be freed
char *read_whole_file(const char *filename);

// Returns the size of the loaded program
size_t load_program(byte *memory, size_t memory_size, const char *filename);

void error_file_doesnot_exist(const char *filename);

#endif
