#ifndef __COMMON_ERROR_H
#define __COMMON_ERROR_H

typedef enum {
    SUCCESS,
    FILE_DOESNOT_EXIST,

    // Args: const char *section_name
    SECTION_NOT_FOUND,

    // Args: Token unexpected_token, TokenType expected_token_type
    UNEXPECTED_TOKEN,
    // Args: Token unexpected_token, size_t types_count, [TokenType...]
    UNEXPECTED_TOKEN_IN_LIST,
    // Args: Token unexpected_token, vector(TokenType) types
    UNEXPECTED_TOKEN_IN_VEC,
    // Args: Span pos
    UNEXPECTED_COMMA,

    // Args: Span pos
    MISSED_LABEL,

    // Args: TokenType invalid_op, TokenType valid_op, Span pos
    INVALID_OPERAND,
    // Args: TokenType invalid_op, Span pos, size_t valid_types_cnt, ...
    INVALID_OPERAND_IN_LIST,
    // Args: const char *reg_name, Span pos
    UNKNOWN_REGISTER,

    // Args: Span span
    INCORRECT_CHARACTER,
} ErrorType;

// Displays error and exits. See also ErrorType for more information about additional args
void throw_error(ErrorType type, ...);


#endif
