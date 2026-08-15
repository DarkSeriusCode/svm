#include "lexer_base.h"
#include <ctype.h>
#include <common/io.h>

Token new_token(int type, const char *value, Span span) {
    return (Token) { type, strdup(value), span };
}

Token copy_token(Token tok) {
    Token t = {
        .value = strdup(tok.value),
        .type = tok.type,
        .span = tok.span,
    };
    return t;
}

void free_token(void *lexem) {
    free((void *)((Token*)lexem)->value);
}

Span lexer_calc_span(Span span, size_t buff_size) {
    return (Span) { span.column - buff_size, span.line, buff_size };
}

Lexer new_lexer(const char *file_name) {
    vector(Token) token_buffer = NULL;
    vector_reserve(token_buffer, 3);
    char *source = read_whole_file(file_name);
    return (Lexer){
        .i = 1,
        .c = source[0],
        .source_file = source,
        .source_len = strlen(source),
        .current_span = (Span){ 1, 1, 0 },
        .file_name = file_name,
        .token_buffer = token_buffer,
        .buffer = NULL,
    };
}

void lexer_forward(Lexer *lexer) {
    if (lexer->i < lexer->source_len) {
        lexer->c = lexer->source_file[lexer->i++];
    }
    switch (lexer->c) {
        case '\n':
            lexer->current_span.line++;
            lexer->current_span.column = 1;
            break;
        default:
            lexer->current_span.column++;
    }
}

void lexer_skip_whitespaces(Lexer *lexer) {
    if (lexer->i >= strlen(lexer->source_file)) return;
    while (isspace(lexer->c)) lexer_forward(lexer);
}

void lexer_skip_comment(Lexer *lexer, const char *comment) {
    while (strncmp(lexer->source_file + lexer->i -1, comment, strlen(comment)) == 0) {
        while (lexer->c != 10) lexer_forward(lexer);
        lexer_forward(lexer);
    }
}

void free_lexer(void *lexer_ptr) {
    Lexer lexer = *(Lexer *)lexer_ptr;
    free((void *)lexer.source_file);
    free_vector(&lexer.token_buffer);
    free_string(&lexer.buffer);
}
