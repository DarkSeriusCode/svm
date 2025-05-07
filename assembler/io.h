#ifndef __ASM_IO_H
#define __ASM_IO_H

#include "image.h"
#include "lexer.h"
#include "parser.h"

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

const char *token_type_to_str(TokenType type);

void print_line_with_underline(const char *filename, Span span, Color color);

void print_span(Span span);
void print_token(Token tok);
void print_image(Image img);

void dump_image(Image image, const char *filename);

// ------------------------------------------------------------------------------------------------

void error_unexpected_token(Token unexpected_token, TokenType expected_token_type);
void error_unexpected_token_in_args(Token unexpected_token, size_t types_count, ...);
void error_unexpected_token_in_vec(Token unexpected_token, vector(TokenType) types);
void error_unexpected_comma(Span pos);
void error_missed_label(TokenType instead_of_label, Span pos);
void error_invalid_operand_in_vec(TokenType invalid_op, Span pos, vector(TokenType) valid_types);
void error_unknown_register(const char *reg_name, Span pos);
void error_invalid_character(Span pos);
void error_redefinition(const char *name, Span pos);
void error_invalid_name(const char *name, const char *name_of_what, Span pos);
void error_negative_alignment_size(Span pos);
void error_unresolved_name(Symbol name);
void error_entry_point_with_decls(void);

void warning_number_out_of_bounds(long num, long lower_bound, long upper_bound, Span pos);
void warning_empty_label(Label lbl);

void note_zero_alignment(Span pos);

#endif
