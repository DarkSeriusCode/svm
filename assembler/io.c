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
        case TOKEN_EOF:         return "<EOF>";
        default:                return "[undefined]";
    }
}

void print_line_with_underline(const char *filename, Span span) {
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
        printf("~");
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

void print_image(Image img) {
    printf("Symbol tabel:\n");
    size_t longest_name_len = 0;
    for (size_t i = 0; i < vector_size(img.sym_table); i++) {
        size_t name_len = strlen(img.sym_table[i].name);
        if (longest_name_len < name_len) {
            longest_name_len = name_len;
        }
    }
    foreach(Symbol, symbol, img.sym_table) {
        printf("%s", symbol->name);
        for (size_t j = strlen(symbol->name); j < longest_name_len; j++) {
            printf(".");
        }
        printf(":    0x%04x %s", symbol->address, (symbol->is_resolved) ? "" : "(unresolved)");
        printf("\n");
    }

    printf("\nHex:\n");
    for (size_t i = 0; i < vector_size(img.data); i++) {
        if (i > 0 && i % 16 == 0) {
            printf("\n");
        }
        printf("%02x ", img.data[i]);
    }
    printf("\n");
}

void print_instr(Instr instr) {
    printf("%s ", instr.name);
    for (size_t i = 0; i < vector_size(instr.ops); i++) {
        if (i != vector_size(instr.ops) - 1) {
            printf(", ");
        }
    }
    printf("\n");
}

void print_decl(Decl decl) {
    printf("%s %s\n", decl.kind.value, decl.value.value);
}

void print_label(Label lbl) {
    if (lbl.name == NULL) {
        printf("No label\n");
        return;
    }
    printf("Label `%s`%s:\n", lbl.name, lbl.is_data ? " (data)" : "");
    if (vector_size(lbl.instructions) == 0) {
        printf("No content\n");
        return;
    }
    if (!lbl.is_data) {
        foreach(Instr, instr, lbl.instructions) {
            printf("    ");
            print_instr(*instr);
        }
    } else {
        foreach(Decl, decl, lbl.declarations) {
            printf("    ");
            print_decl(*decl);
        }
    }
    printf("\n");
}

void dump_image(Image image, const char *filename) {
    FILE *fp = fopen(filename, "wb");
    fwrite(image.data, vector_size(image.data), sizeof(byte), fp);
    fclose(fp);
}

//-------------------------------------------------------------------------------------------------

static void print_basic_error_message(Span pos, const char *header, const char *footer) {
    printf("In %s:", INPUT_FILE_NAME);
    print_span(pos);
    printf(" %s\n", header);
    print_line_with_underline(INPUT_FILE_NAME, pos);
    if (footer) {
        printf("  %s", footer);
    }
}

void error_section_not_found(const char *section_name) {
    printf("In %s: No section `%s`!\n", INPUT_FILE_NAME, section_name);
    exit(EXIT_FAILURE);
}

void error_unexpected_token(Token unexpected_token, TokenType expected_token_type) {
    print_basic_error_message(unexpected_token.span, "Syntax error. Unexpected token!", NULL);
    printf("  Expected `%s` but got `%s`",
            token_type_to_str(expected_token_type), unexpected_token.value);
    if (unexpected_token.type != TOKEN_UNKNOWN && unexpected_token.type != TOKEN_IDENT
        && unexpected_token.type != TOKEN_INSTR)
    {
        printf(" which is `%s`!", token_type_to_str(unexpected_token.type));
    }
    printf("\n");
    exit(EXIT_FAILURE);
}

void error_unexpected_token_in_args(Token unexpected_token, size_t types_count, ...) {
    va_list args;
    va_start(args, types_count);
    print_basic_error_message(unexpected_token.span, "Syntax error. Unexpected token!", "Expected");
    for (size_t i = 0; i < types_count; i++) {
        printf(" `%s`", token_type_to_str(va_arg(args, TokenType)));
        if (i != types_count - 1) {
            printf(" ");
            printf("or");
        }
    }
    printf(" but got `%s`", unexpected_token.value);
    if (unexpected_token.type != TOKEN_UNKNOWN && unexpected_token.type != TOKEN_IDENT
        && unexpected_token.type != TOKEN_INSTR)
    {
        printf(" which is `%s`!", token_type_to_str(unexpected_token.type));
    }
    printf("\n");
    va_end(args);
    exit(EXIT_FAILURE);
}

void error_unexpected_token_in_vec(Token unexpected_token, vector(TokenType) types) {
    print_basic_error_message(unexpected_token.span, "Syntax error. Unexpected token!", "Expected");
    for (size_t i = 0; i < vector_size(types); i++) {
        printf(" `%s`", token_type_to_str(types[i]));
        if (i != vector_size(types) - 1) {
            printf(" ");
            printf("or");
        }
    }
    printf(" but got `%s`", unexpected_token.value);
    if (unexpected_token.type != TOKEN_UNKNOWN && unexpected_token.type != TOKEN_IDENT
        && unexpected_token.type != TOKEN_INSTR)
    {
        printf(" which is `%s`!", token_type_to_str(unexpected_token.type));
    }
    printf("\n");
    exit(EXIT_FAILURE);
}

void error_unexpected_comma(Span pos) {
    print_basic_error_message(pos, "Syntax error. Unexpected comma!", NULL);
    exit(EXIT_FAILURE);
}

void error_missed_label(TokenType instead_of_label, Span pos) {
    print_basic_error_message(pos, "Syntax error. Missed label!", NULL);
    if (instead_of_label == TOKEN_IDENT) {
        printf("  It looks like you forgot ':' in the end of label name.\n");
    } else if (instead_of_label == TOKEN_INSTR) {
        printf("  Instruction cannot be outside a label. Try to write `label:` above this line\n");
    }
    exit(EXIT_FAILURE);
}

void error_invalid_operand(TokenType invalid_op, TokenType valid_op, Span pos) {
    print_basic_error_message(pos, "Invalid instruction operand!", NULL);
    printf("  Expected `%s` but got `%s`!\n",
            token_type_to_str(valid_op), token_type_to_str(invalid_op));
    exit(EXIT_FAILURE);
}

void error_invalid_operand_in_vec(TokenType invalid_op, Span pos, vector(TokenType) valid_types) {
    print_basic_error_message(pos, "Invalid instruction operand!", "Expected");
    for (size_t i = 0; i < vector_size(valid_types); i++) {
        printf(" `%s`", token_type_to_str(valid_types[i]));
        if (i != vector_size(valid_types) - 1) {
            printf(" ");
            printf("or");
        }
    }
    printf(" but got `%s`\n", token_type_to_str(invalid_op));
    exit(EXIT_FAILURE);
}

void error_unknown_register(const char *reg_name, Span pos) {
    print_basic_error_message(pos, "Unknown register!", NULL);
    printf("  Register %s does not exist. Did you spell it right?\n", reg_name);
    exit(EXIT_FAILURE);
}

void error_invalid_character(Span pos) {
    print_basic_error_message(pos, "Syntax error. Invalid character", NULL);
    exit(EXIT_FAILURE);
}

void error_redefinition(const char *name, Span pos) {
    printf("In %s:", INPUT_FILE_NAME);
    print_span(pos);
    printf(" Redefinition of name `%s`\n", name);
    print_line_with_underline(INPUT_FILE_NAME, pos);
    exit(EXIT_FAILURE);
}

void error_undefined_identifier(Token ident) {
    printf("In %s:", INPUT_FILE_NAME);
    print_span(ident.span);
    printf(" Name '%s' is not declared!\n", ident.value);
    print_line_with_underline(INPUT_FILE_NAME, ident.span);
    exit(EXIT_FAILURE);
}

void error_missed_bracket(const char *bracket, Span pos) {
    print_basic_error_message(pos, "Syntax error. Missed bracket!", NULL);
    printf(" Try to add '%s'\n", bracket);
    exit(EXIT_FAILURE);
}

void error_invalid_name(const char *name, const char *name_of_what, Span span) {
    printf("In %s:", INPUT_FILE_NAME);
    print_span(span);
    printf(" Invalid name of %s: \"%s\"\n", name_of_what, name);
    print_line_with_underline(INPUT_FILE_NAME, span);
    exit(EXIT_FAILURE);
}

void error_negative_alignment_size(Span pos) {
    print_basic_error_message(pos, "Invalid value. You cannot use a negative alignment!", NULL);
    exit(EXIT_FAILURE);
}

void error_unresolved_name(Symbol name) {
    printf("In %s:", INPUT_FILE_NAME);
    printf(" Cannot find definition of name `%s`!\n", name.name);
    exit(EXIT_FAILURE);
}

void error_entry_point_with_data(void) {
    printf("In %s: _main label cannot contain data!\n", INPUT_FILE_NAME);
    exit(EXIT_FAILURE);
}

void error_usage_of_undefined_label(const char *label_name, Span pos) {
    printf("In %s:", INPUT_FILE_NAME);
    print_span(pos);
    printf(" Usage of undefined name `%s`!\n", label_name);
    print_line_with_underline(INPUT_FILE_NAME, pos);
    exit(EXIT_FAILURE);
}

// ------------------------------------------------------------------------------------------------

void warning_number_out_of_bounds(long num, long lower_bound, long upper_bound, Span pos) {
    printf("In %s:", INPUT_FILE_NAME);
    print_span(pos);
    printf(" warning: Narrowing convertion is possible. Number %ld is out of bounds [%ld;%ld]\n", num,
            lower_bound, upper_bound);
    print_line_with_underline(INPUT_FILE_NAME, pos);
}

void warning_empty_label(Label lbl) {
    printf("In %s:", INPUT_FILE_NAME);
    print_span(lbl.span);
    printf(" warning: Empty label (%s) will be ignored!\n", lbl.name);
}

// ------------------------------------------------------------------------------------------------

void note_zero_alignment(Span pos) {
    printf("In %s:", INPUT_FILE_NAME);
    print_span(pos);
    printf(" note: You use a zero alignment here.\n");
    print_line_with_underline(INPUT_FILE_NAME, pos);
}
