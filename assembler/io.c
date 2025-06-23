#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include "io.h"
#include "common/io.h"
#include "common/utils.h"

extern const char *INPUT_FILE_NAME;

const char *token_type_to_str(TokenType type) {
    switch (type) {
        case TOKEN_UNKNOWN:     return "<UNKNOWN>";
        case TOKEN_IDENT:       return "identifier";
        case TOKEN_LABEL:       return "label";
        case TOKEN_INSTR:       return "instruction";
        case TOKEN_REG:         return "register";
        case TOKEN_COMMA:       return "comma";
        case TOKEN_NUMBER:      return "number";
        case TOKEN_DECL:        return "declaration";
        case TOKEN_STRING:      return "string";
        case TOKEN_CMP:         return "cmp";
        case TOKEN_DIRECTIVE:   return "directive";
        case TOKEN_EOF:         return "<EOF>";
        default:                return "[undefined]";
    }
}

void print_line_with_underline(const char *filename, Span span, Color color) {
    char *file_content = read_whole_file(filename);
    long new_lines = 0;
    long lines = (long)span.line;
    while (new_lines < max(lines - 3, 0) && *file_content != '\0') {
        if (*file_content == 10) {
            new_lines++;
        }
        file_content++;
    }
    char *line = strsep(&file_content, "\n");
    char offset[32];
    for (long i = lines - 3; i < lines; i++) {
        if (i < 0) continue;
        sprintf(offset, "%lu | ", i + 1);
        printf("%s%s\n", offset, line);

        line = strsep(&file_content, "\n");
    }
    for (size_t i = 0; i < span.column - 1 + strlen(offset); i++) {
        printf(" ");
    }
    for (size_t i = 0; i < span.len; i++) {
        style(STYLE_BOLD);
        printf_with_color(color, "~");
    }
    printf("\n");
}

void print_span(Span span) {
    printf("%lu:%lu", span.line, span.column);
}

void print_token(Token tok) {
    printf("%s: `%s` at (%lu, %lu)\n", token_type_to_str(tok.type), tok.value,
                                     tok.span.column, tok.span.line);
}

// TODO: Add dirs
void print_program(Program prog) {
    printf("Symbol tabel:\n");
    size_t longest_name_len = 0;
    for (size_t i = 0; i < vector_size(prog.sym_table); i++) {
        size_t name_len = strlen(prog.sym_table[i].name);
        if (longest_name_len < name_len) {
            longest_name_len = name_len;
        }
    }
    foreach(Symbol, symbol, prog.sym_table) {
        printf("%s", symbol->name);
        for (size_t j = strlen(symbol->name); j < longest_name_len; j++) {
            printf(".");
        }
        printf(":    0x%04x", symbol->address);
        if (!symbol->is_resolved) {
            style(STYLE_BOLD);
            printf_red(" (unresolved)");
        }
        size_t unresolved_usages_count = vector_size(symbol->unresolved_usages);
        if (unresolved_usages_count != 0) {
            printf_cyan("  [used in %ld place%s", unresolved_usages_count,
                    (unresolved_usages_count == 1) ? "]" : "s]");
        }
        printf("\n");
    }
    printf("\n");
}

//-------------------------------------------------------------------------------------------------

static void print_message(const char *lvl, const char *color, Span pos, const char *header, ...) {
    va_list args;
    va_start(args, header);

    style(STYLE_BOLD);
    printf("%s:", INPUT_FILE_NAME);
    style(STYLE_NONE);
    print_span(pos);
    printf(": ");

    style(STYLE_BOLD);
    printf_with_color(color, "%s:", lvl);
    style(STYLE_NONE);
    printf(" ");
    vprintf(header, args);
    printf("\n");

    print_line_with_underline(INPUT_FILE_NAME, pos, color);
    va_end(args);
}
#define print_error(pos, header, ...) \
    print_message("error", COLOR_RED, pos, header, __VA_ARGS__)
#define print_note(pos, header, ...) \
    print_message("note", COLOR_BLUE, pos, header, __VA_ARGS__)
#define print_warning(pos, header, ...) \
    print_message("warning", COLOR_YELLOW, pos, header, __VA_ARGS__)

void error_unexpected_token(Token unexpected_token, TokenType expected_token_type) {
    print_error(unexpected_token.span, "Syntax error. Unexpected token!", NULL);
    printf("  Expected ");
    style(STYLE_BOLD);
    printf_green("%s", token_type_to_str(expected_token_type));
    printf(" but got `");
    style(STYLE_BOLD);
    printf_red("%s", unexpected_token.value);
    printf("`");
    if (unexpected_token.type != TOKEN_EOF) {
        printf(" which is ");
        style(STYLE_BOLD);
        printf_yellow("%s", token_type_to_str(unexpected_token.type));
    }
    printf("!\n");
    exit(EXIT_FAILURE);
}

void error_unexpected_token_in_args(Token unexpected_token, size_t types_count, ...) {
    va_list args;
    va_start(args, types_count);
    print_error(unexpected_token.span, "Syntax error. Unexpected token!", NULL);
    printf("  Expected");
    for (size_t i = 0; i < types_count; i++) {
        style(STYLE_BOLD);
        printf_green(" %s", token_type_to_str(va_arg(args, TokenType)));
        if (i != types_count - 1) {
            printf(" ");
            printf("or");
        }
    }
    printf(" but got `");
    style(STYLE_BOLD);
    printf_red("%s", unexpected_token.value);
    printf("`");
    if (unexpected_token.type != TOKEN_EOF) {
        printf(" which is ");
        style(STYLE_BOLD);
        printf_yellow("%s", token_type_to_str(unexpected_token.type));
    }
    printf("!\n");
    va_end(args);
    exit(EXIT_FAILURE);
}

void error_unexpected_token_in_vec(Token unexpected_token, vector(TokenType) types) {
    print_error(unexpected_token.span, "Syntax error. Unexpected token!", "Expected");
    for (size_t i = 0; i < vector_size(types); i++) {
        style(STYLE_BOLD);
        printf_green(" %s", token_type_to_str(types[i]));
        if (i != vector_size(types) - 1) {
            printf(" ");
            printf("or");
        }
    }
    printf(" but got `");
    style(STYLE_BOLD);
    printf_red("%s", unexpected_token.value);
    printf("`");
    if (unexpected_token.type != TOKEN_EOF) {
        printf(" which is ");
        style(STYLE_BOLD);
        printf_yellow("%s", token_type_to_str(unexpected_token.type));
    }
    printf("!\n");
    exit(EXIT_FAILURE);
}

void error_unexpected_comma(Span pos) {
    print_error(pos, "Syntax error. Unexpected comma!", NULL);
    exit(EXIT_FAILURE);
}

void error_missed_label(TokenType instead_of_label, Span pos) {
    print_error(pos, "Syntax error. Missed label!", NULL);
    if (instead_of_label == TOKEN_IDENT) {
        printf("  It looks like you forgot '");
        style(STYLE_BOLD);
        printf_green(":");
        printf("' in the end of label name.\n");
    } else if (instead_of_label == TOKEN_INSTR) {
        printf("  Instruction cannot be outside a label. Try to write `");
        style(STYLE_BOLD);
        printf_green("label:");
        printf("` above this line\n");
    }
    exit(EXIT_FAILURE);
}

void error_invalid_operand_in_vec(TokenType invalid_op, Span pos, vector(TokenType) valid_types) {
    print_error(pos, "Invalid operand!", NULL);
    printf("  Expected");
    for (size_t i = 0; i < vector_size(valid_types); i++) {
        style(STYLE_BOLD);
        printf_green(" %s", token_type_to_str(valid_types[i]));
        if (i != vector_size(valid_types) - 1) {
            printf(" or");
        }
    }
    printf(" but got ");
    style(STYLE_BOLD);
    printf_red("%s", token_type_to_str(invalid_op));
    printf("\n");
    exit(EXIT_FAILURE);
}

void error_unknown_register(const char *reg_name, Span pos) {
    print_error(pos, "Unknown register!", NULL);
    printf("  Register %s does not exist. Did you spell it right?\n", reg_name);
    exit(EXIT_FAILURE);
}

void error_invalid_character(Span pos) {
    print_error(pos, "Syntax error. Invalid character", NULL);
    exit(EXIT_FAILURE);
}

void error_redefinition(const char *name, Span pos) {
    print_error(pos, "Redefinition of name `%s`!", name);
    exit(EXIT_FAILURE);
}

void error_invalid_name(const char *name, const char *name_of_what, Span pos) {
    print_error(pos, "Syntax error. Invalid %s name: `%s`!", name_of_what, name);
    exit(EXIT_FAILURE);
}

void error_negative_alignment_size(Span pos) {
    print_error(pos, "Invalid value. You cannot use a negative alignment!", NULL);
    exit(EXIT_FAILURE);
}

void error_unresolved_name(Symbol name) {
    print_error(name.unresolved_usages[0].pos, "Cannot find definition of name `%s`!", name.name);
    exit(EXIT_FAILURE);
}

void error_entry_point_with_decls(void) {
    style(STYLE_BOLD);
    printf("%s: ", INPUT_FILE_NAME);
    printf_red("error: ");
    printf("Label "ENTRY_POINT_NAME" cannot contain declarations!\n");
    exit(EXIT_FAILURE);
}

void error_empty_file(void) {
    style(STYLE_BOLD);
    printf("%s: ", INPUT_FILE_NAME);
    printf_red("error: ");
    printf("Empty file! Try to write:\n");
    printf_green(";; This program doesn't do anything special but at least it compiles ;-;\n"
ENTRY_POINT_NAME":\n\t;; your code goes here\nret\n")
    exit(EXIT_FAILURE);
}

void error_no_entry(void) {
    style(STYLE_BOLD);
    printf("%s: ", INPUT_FILE_NAME);
    printf_red("error: ");
    printf("There's no "ENTRY_POINT_NAME" label! Try to write:\n");
    printf_green(ENTRY_POINT_NAME":\n\t;; your code goes here\nret\n");
    exit(EXIT_FAILURE);
}

void error_unknown_directive(Token directive) {
    print_error(directive.span, "Unknown directive `%s`!", directive.value);
    exit(EXIT_FAILURE);
}

// ------------------------------------------------------------------------------------------------

void warning_number_out_of_bounds(long num, long lower_bound, long upper_bound, Span pos) {
    print_warning(pos, "Narrowing convertion is possible. Number %ld is out of bounds [%ld;%ld]",
                  num, lower_bound, upper_bound);
}

// ------------------------------------------------------------------------------------------------

void note_zero_alignment(Span pos) {
    print_note(pos, "You use a zero alignment here.", NULL);
}
