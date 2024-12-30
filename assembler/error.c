#include "error.h"
#include "io.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

extern const char *INPUT_FILE_NAME;

static void print_basic_error_message(Span pos, const char *header, const char *footer) {
    printf("In %s:", INPUT_FILE_NAME);
    print_span(pos);
    printf(" %s\n\n", header);
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
    printf("  Expected `%s` but got %s which is `%s`!\n",
            token_type_to_str(expected_token_type), unexpected_token.value,
            token_type_to_str(unexpected_token.type));
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
    printf(" but got %s which is `%s`!\n", unexpected_token.value,
           token_type_to_str(unexpected_token.type));
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
    printf(" but got %s which is `%s`!\n", unexpected_token.value,
           token_type_to_str(unexpected_token.type));
    exit(EXIT_FAILURE);
}

void error_unexpected_comma(Span pos) {
    print_basic_error_message(pos, "Syntax error. Unexpected comma!", NULL);
    exit(EXIT_FAILURE);
}

void error_missed_label(Span pos) {
    print_basic_error_message(pos,
        "Syntax error. Missed label!",
        "Instruction cannot be outside a label. Try to write `label_name:` above this line"
    );
    exit(EXIT_FAILURE);
}

void error_invalid_operand(TokenType invalid_op, TokenType valid_op, Span pos) {
    print_basic_error_message(pos, "Invalid instruction operand!", NULL);
    printf("  Expected `%s` but got `%s`!\n",
            token_type_to_str(valid_op), token_type_to_str(invalid_op));
    exit(EXIT_FAILURE);
}

void error_invalid_operand_in_args(TokenType invalid_op, Span pos, size_t types_count, ...) {
    va_list args;
    va_start(args, types_count);
    print_basic_error_message(pos, "Invalid instruction operand!", "Expected");
    for (size_t i = 0; i < types_count; i++) {
        printf(" `%s`", token_type_to_str(va_arg(args, TokenType)));
        if (i != types_count - 1) {
            printf(" ");
            printf("or");
        }
    }
    printf(" but got `%s`\n", token_type_to_str(invalid_op));
    va_end(args);
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
