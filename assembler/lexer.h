#ifndef __TASM_LEXER_H
#define __TASM_LEXER_H

#include <stddef.h>
#include <stdbool.h>

typedef enum {
    TOKEN_UNKNOWN,
    TOKEN_SECTION,
    TOKEN_IDENT,
    TOKEN_LABEL,
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
    size_t column, line;
} Span;

typedef struct {
    TokenType type;
    const char *value;
    Span span;
} Token;

Token new_token(TokenType type, const char *value, Span span);
const char *token_type_to_str(TokenType type);
void free_token(Token lexem);

bool is_ident(const char *buffer);
bool is_label(const char *buffer);
bool is_instr(const char *buffer);
bool is_reg(const char *buffer);
bool is_number(const char *buffer);

// ------------------------------------------------------------------------------------------------

typedef struct {
    char *source_file;
    size_t i;
    char c;
    Span current_span;
} Lexer;

Lexer new_lexer(const char *file_name);
void lexer_advice(Lexer *lexer);
Token lex_string(Lexer *lexer);
void lexer_skip_whitespaces(Lexer *lexer);
void lexer_skip_comment(Lexer *lexer);
Token lexer_get_next_token(Lexer *lexer);
void free_lexer(Lexer lexer);

#endif
