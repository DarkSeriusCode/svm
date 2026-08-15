// FIX: rewrite lexer
#ifndef __ASM_LEXER_H
#define __ASM_LEXER_H

#include <stddef.h>
#include <stdbool.h>
#include "common/lexer_base.h"

typedef enum {
    TOKEN_UNKNOWN = 0,
    TOKEN_IDENT,
    TOKEN_LABEL,
    TOKEN_INSTR,
    TOKEN_REG,
    TOKEN_COMMA,
    TOKEN_NUMBER,
    TOKEN_DECL,
    TOKEN_STRING,
    TOKEN_CMP,
    TOKEN_DIRECTIVE,
    TOKEN_EOF,
} TokenType;

bool is_reg(const char *buffer);
bool is_instr(const char *buffer);
bool is_number(const char *buffer);
bool is_cmp(const char *buffer);
// Checks buffer for incorrect chars, returns a pointer to wrong char in buffer.
// Returns NULL if everything is correct
const char *is_incorrect(const char *buffer);

Token lex_string(Lexer *lexer);
Token make_nonterm(const char *buffer, Span token_span);
Token lexer_get_next_token(Lexer *lexer);

#endif
