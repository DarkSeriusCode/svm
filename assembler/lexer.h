#ifndef __ASM_LEXER_H
#define __ASM_LEXER_H

#include <stddef.h>
#include <stdbool.h>
#include <common/vector.h>

typedef enum {
    TOKEN_UNKNOWN = 0,
    TOKEN_SECTION,
    TOKEN_IDENT,
    TOKEN_COLON,
    TOKEN_INSTR,
    TOKEN_REG,
    TOKEN_COMMA,
    TOKEN_NUMBER,
    TOKEN_END,
    TOKEN_DECL,
    TOKEN_STRING,
    TOKEN_EOF,
} TokenType;

typedef struct {
    size_t column, line, len;
} Span;

typedef struct {
    TokenType type;
    const char *value;
    Span span;
} Token;

Token new_token(TokenType type, const char *value, Span span);
const char *token_type_to_str(TokenType type);
void free_token(void *lexem);

bool is_reg(const char *buffer);
bool is_instr(const char *buffer);
bool is_number(const char *buffer);
// Checks buffer for incorrect chars, returns a pointer to wrong char in buffer.
// Returns NULL if everything is correct
const char *is_incorrect(const char *buffer);

// ------------------------------------------------------------------------------------------------

typedef struct {
    char *source_file;
    const char *file_name;
    size_t i;
    char c;
    Span current_span;
    vector(Token) token_buffer;
} Lexer;

Lexer new_lexer(const char *file_name);
void lexer_advice(Lexer *lexer);
Token lex_string(Lexer *lexer);
void lexer_skip_whitespaces(Lexer *lexer);
void lexer_skip_comment(Lexer *lexer);
Token make_nonterm(Lexer *lexer, const char *buffer, Span token_span);
Token lexer_get_next_token(Lexer *lexer);
void free_lexer(void *lexer);

#endif
