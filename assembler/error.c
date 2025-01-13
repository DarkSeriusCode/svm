#include "error.h"
#include "io.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

extern const char *INPUT_FILE_NAME;

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

// ------------------------------------------------------------------------------------------------

void warning_number_out_of_bounds(long num, long lower_bound, long upper_bound, Span pos) {
    printf("In %s:", INPUT_FILE_NAME);
    print_span(pos);
    printf(" Narrowing convertion is possible. Number %ld is out of bounds [%ld;%ld]\n", num,
            lower_bound, upper_bound);
    print_line_with_underline(INPUT_FILE_NAME, pos);
}

void warning_wrong_ident_size(Token ident, Decl decl, size_t expected_size) {
    assert(expected_size == 1 || expected_size == 2);
    print_basic_error_message(ident.span, "Used identifier with wrong size!", NULL);
    char *size_should_be = "byte";
    char *wrong_size = "word";
    if (expected_size == 2) {
        size_should_be = "word";
        wrong_size = "byte";
    }
    printf("  Size should be %s instead of %s!\n", size_should_be, wrong_size);
    printf("\n  Identifier '%s' is defined here:\n", ident.value);
    print_line_with_underline(INPUT_FILE_NAME, decl.pos);
    printf("\n");
}
