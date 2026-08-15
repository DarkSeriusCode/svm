#ifndef __COMMON_LEXER_BASE_H
#define __COMMON_LEXER_BASE_H

#include <stddef.h>
#include <common/vector.h>
#include <common/str.h>

typedef struct {
    size_t column, line, len;
} Span;

typedef struct {
    int type;
    const char *value;
    Span span;
} Token;

Token new_token(int type, const char *value, Span span);
Token copy_token(Token tok);
void free_token(void *token);

typedef struct {
    char *source_file;
    size_t source_len;
    const char *file_name;
    size_t i;
    char c;
    Span current_span;
    vector(Token) token_buffer;
    string buffer;
} Lexer;

Lexer new_lexer(const char *file_name);
void lexer_forward(Lexer *lexer);
Span lexer_calc_span(Span span, size_t buff_size);
void lexer_skip_whitespaces(Lexer *lexer);
void lexer_skip_comment(Lexer *lexer, const char *comment_beginning);
void free_lexer(void *lexer);

#endif
