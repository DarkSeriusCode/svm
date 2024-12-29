#include "error.h"
#include "assembler/lexer.h"
#include "common/io.h"
#include "common/vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

extern const char *INPUT_FILE_NAME;

void throw_error(ErrorType type, ...) {
    va_list args;
    va_start(args, type);

    switch (type) {
        case SUCCESS: break;
        case FILE_DOESNOT_EXIST: {
            printf("File not found: %s\n", INPUT_FILE_NAME);
        }; break;

        case SECTION_NOT_FOUND: {
            char *section_name = va_arg(args, char *);
            printf("In %s: No section `%s`!\n", INPUT_FILE_NAME, section_name);
        }; break;

        case UNEXPECTED_TOKEN: {
            Token unexpected_token = va_arg(args, Token);
            TokenType expected_type = va_arg(args, TokenType);
            printf("In %s:", INPUT_FILE_NAME);
            print_span(unexpected_token.span);
            printf(" Syntax error. Unexpected token!\n\n");
            print_line_with_underline(INPUT_FILE_NAME, unexpected_token.span);
            printf("  Expected `%s` but got %s which is `%s`!\n",
                    token_type_to_str(expected_type), unexpected_token.value,
                    token_type_to_str(unexpected_token.type));
        }; break;

        case UNEXPECTED_TOKEN_IN_LIST: {
            Token unexpected_token = va_arg(args, Token);
            size_t types_count = va_arg(args, size_t);
            printf("In %s:", INPUT_FILE_NAME);
            print_span(unexpected_token.span);
            printf(" Syntax error. Unexpected token!\n\n");
            print_line_with_underline(INPUT_FILE_NAME, unexpected_token.span);
            printf("  Expected");
            for (size_t i = 0; i < types_count; i++) {
                printf(" `%s`", token_type_to_str(va_arg(args, TokenType)));
                if (i != types_count - 1) {
                    printf(" ");
                    printf("or");
                }
            }
            printf(" but got %s which is `%s`!\n", unexpected_token.value,
                   token_type_to_str(unexpected_token.type));
        }; break;
        case UNEXPECTED_TOKEN_IN_VEC: {
            Token unexpected_token = va_arg(args, Token);
            vector(TokenType) types = va_arg(args, vector(TokenType));
            printf("In %s:", INPUT_FILE_NAME);
            print_span(unexpected_token.span);
            printf(" Syntax error. Unexpected token!\n\n");
            print_line_with_underline(INPUT_FILE_NAME, unexpected_token.span);
            printf("  Expected");
            for (size_t i = 0; i < vector_size(types); i++) {
                printf(" `%s`", token_type_to_str(types[i]));
                if (i != vector_size(types) - 1) {
                    printf(" ");
                    printf("or");
                }
            }
            printf(" but got %s which is `%s`!\n", unexpected_token.value,
                   token_type_to_str(unexpected_token.type));
        }; break;
        case UNEXPECTED_COMMA: {
            Span pos = va_arg(args, Span);
            printf("In %s:", INPUT_FILE_NAME);
            print_span(pos);
            printf(" Syntax error. Unexpected comma!\n\n");
            print_line_with_underline(INPUT_FILE_NAME, pos);
        }; break;

        case MISSED_LABEL: {
            Span positon = va_arg(args, Span);

            printf("In %s:", INPUT_FILE_NAME);
            print_span(positon);
            printf(" Syntax error. Missed label!\n\n");
            print_line_with_underline(INPUT_FILE_NAME, positon);
            printf("  Instruction cannot be outside a label. Try to write `label_name:` above this line\n");
        }; break;

        case INVALID_OPERAND: {
            TokenType invalid_op = va_arg(args, TokenType);
            TokenType valid_op = va_arg(args, TokenType);
            Span position = va_arg(args, Span);
            printf("In %s:", INPUT_FILE_NAME);
            print_span(position);
            printf(" Invalid instruction operand!\n\n");
            print_line_with_underline(INPUT_FILE_NAME, position);
            printf("  Expected `%s` but got `%s`!\n",
                    token_type_to_str(valid_op), token_type_to_str(invalid_op));
        }; break;

        case INVALID_OPERAND_IN_LIST: {
            TokenType invalid_op = va_arg(args, TokenType);
            Span position = va_arg(args, Span);
            size_t types_count = va_arg(args, size_t);
            printf("In %s:", INPUT_FILE_NAME);
            print_span(position);
            printf(" Invalid instruction operand!\n\n");
            print_line_with_underline(INPUT_FILE_NAME, position);
            printf("  Expected");
            for (size_t i = 0; i < types_count; i++) {
                printf(" `%s`", token_type_to_str(va_arg(args, TokenType)));
                if (i != types_count - 1) {
                    printf(" ");
                    printf("or");
                }
            }
            printf(" but got `%s`\n", token_type_to_str(invalid_op));
        }; break;

        case UNKNOWN_REGISTER: {
            char *reg_name = va_arg(args, char *);
            Span pos = va_arg(args, Span);
            printf("In %s:", INPUT_FILE_NAME);
            print_span(pos);
            printf(" Unknown register!\n\n");
            print_line_with_underline(INPUT_FILE_NAME, pos);
            printf("  Register %s does not exist. Did you spell it right?\n", reg_name);
        }; break;

        case INCORRECT_CHARACTER: {
            Span span = va_arg(args, Span);
            printf("In %s:", INPUT_FILE_NAME);
            print_span(span);
            printf(" Syntax error. Incorrect character\n\n");
            print_line_with_underline(INPUT_FILE_NAME, span);
        }; break;
    }
    va_end(args);
    exit(EXIT_FAILURE);
}
