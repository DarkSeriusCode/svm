#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "lexer.h"
#include "io.h"
#include "common/arch.h"
#include "common/io.h"
#include "common/utils.h"

Token new_token(TokenType type, const char *value, Span span) {
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

bool is_reg(const char *buffer) {
    if (string_in_args(buffer, 3, "sp", "cf", "ip")) return true;
    if (buffer[0] != 'r') return false;
    if (buffer[1] != '\0' && is_number(buffer + 1)) return true;
    return false;
}

bool is_ident(const char *buffer) {
    if (isdigit(buffer[0])) return false;
    for (size_t i = 0; i < strlen(buffer); i++) {
        if (!(buffer[i] == '_' || (buffer[i] == '.' && i == 0) || isalnum(buffer[i]))) return false;
    }
    return true;
}

bool is_number(const char *buffer) {
    for (size_t i = 0; i < strlen(buffer); i++) {
        if (buffer[i] == '-') continue;
        if (!isdigit(buffer[i])) return false;
    }
    return true;
}

bool is_cmp(const char *buffer) {
    return string_in_args(buffer, 6, "eq", "nq", "lt", "lq", "gt", "gq");
}

const char *is_incorrect(const char *buffer) {
    for (const char *c = buffer; *c != '\0'; c++)
        if (*c != '#' && *c != '-' && *c != '.' && *c != '_' && !isalnum(*c)) return c;
    return NULL;
}

// ------------------------------------------------------------------------------------------------

static Span calc_span(Span span, size_t buff_size) {
    return (Span) { span.column - buff_size, span.line, buff_size };
}

Lexer new_lexer(const char *file_name) {
    vector(Token) token_buffer = NULL;
    vector_reserve(token_buffer, 3);
    char *source = read_whole_file(file_name);
    if (strlen(source) == 0) {
        error_empty_file();
    }
    return (Lexer){
        .i = 0,
        .c = 0,
        .source_file = source,
        .current_span = (Span){ 1, 1, 0 },
        .file_name = file_name,
        .token_buffer = token_buffer,
        .buffer = NULL,
    };
}

void lexer_forward(Lexer *lexer) {
    if (lexer->i < strlen(lexer->source_file)) {
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

Token lex_string(Lexer *lexer) {
    vector(char) buff = NULL;
    lexer_forward(lexer);
    while (lexer->c != '"') {
        vector_push_back(buff, lexer->c);
        lexer_forward(lexer);
    }
    vector_push_back(buff, '\0');
    Token str = new_token(TOKEN_STRING, buff, calc_span(lexer->current_span, strlen(buff) + 2));
    free_vector(&buff);
    return str;
}

Token lex_directive(Lexer *lexer) {
    Token dir_ident = lexer_get_next_token(lexer);
    Span span = calc_span(lexer->current_span, strlen(dir_ident.value) + 1);
    span.column--;
    Token dir = new_token(TOKEN_DIRECTIVE, dir_ident.value, span);
    if (!in_directive_set(dir.value)) {
        error_unknown_directive(dir);
    }
    return dir;
}

void lexer_skip_whitespaces(Lexer *lexer) {
    if (lexer->i >= strlen(lexer->source_file)) return;
    while (isspace(lexer->c)) lexer_forward(lexer);
}

void lexer_skip_comment(Lexer *lexer) {
    while (lexer->c == ';') {
        while (lexer->c != 10) lexer_forward(lexer);
        lexer_forward(lexer);
    }
}

Token make_nonterm(const char *buffer, Span token_span) {
    if (is_ident(buffer))
        return new_token(TOKEN_IDENT, buffer, token_span);
    if (is_number(buffer))
        return new_token(TOKEN_NUMBER, buffer, token_span);
    return new_token(TOKEN_UNKNOWN, buffer, token_span);
}

Token lexer_get_next_token(Lexer *lexer) {
    Span tok_span;
    string_clean(lexer->buffer);
    if (!vector_empty(lexer->token_buffer)) {
        Token tok = lexer->token_buffer[0];
        vector_erase(lexer->token_buffer, 0);
        return tok;
    }
    lexer_forward(lexer);
    do {
        // if the entire line is a comment it will skip it.
        lexer_skip_comment(lexer);
        lexer_skip_whitespaces(lexer);
        if (lexer->i >= strlen(lexer->source_file))
            return new_token(TOKEN_EOF, "EOF", lexer->current_span);
    } while (isspace(lexer->c) || lexer->c == ';');

    while (!isspace(lexer->c)) {
        switch (lexer->c) {
            case '"': return lex_string(lexer);
            case ';': lexer_skip_comment(lexer); continue;
            case ',': {
                Token comma = new_token(TOKEN_COMMA, ",", calc_span(lexer->current_span, 1));
                if (strlen(lexer->buffer) > 0) {
                    vector_push_back(lexer->token_buffer, comma);
                } else {
                    return comma;
                }
            }; break;
            case ':': {
                Span pos = calc_span(lexer->current_span, strlen(lexer->buffer));
                pos.column--;
                if (!is_ident(lexer->buffer)) {
                    error_invalid_name(lexer->buffer, "label", pos);
                }
                return new_token(TOKEN_LABEL, lexer->buffer, pos);
            }; break;
            case '#': return lex_directive(lexer);
            default:
                string_push_char(&lexer->buffer, tolower(lexer->c));
                tok_span = calc_span(lexer->current_span, strlen(lexer->buffer));
        }

        // Terms
        if (string_in_args(lexer->buffer, 5, ".byte", ".word", ".align", ".ascii", ".sizeof"))
            return new_token(TOKEN_DECL, lexer->buffer, tok_span);
        if (is_cmp(lexer->buffer)) {
            return new_token(TOKEN_CMP, lexer->buffer, tok_span);
        }
        if (in_instruction_set(lexer->buffer))
            return new_token(TOKEN_INSTR, lexer->buffer, tok_span);
        if (is_reg(lexer->buffer)) {
            if (!in_register_set(lexer->buffer))
                error_unknown_register(lexer->buffer, tok_span);
            return new_token(TOKEN_REG, lexer->buffer, tok_span);
        }
        const char *incorrect_at = is_incorrect(lexer->buffer);
        if (incorrect_at != NULL) {
            tok_span.column += tok_span.len - 1;
            tok_span.len = 1;
            error_invalid_character(tok_span);
        }
        lexer_forward(lexer);
    }
    return make_nonterm(lexer->buffer, tok_span);
}

void free_lexer(void *lexer_ptr) {
    Lexer lexer = *(Lexer *)lexer_ptr;
    free((void *)lexer.source_file);
    free_vector(&lexer.token_buffer);
    free_string(&lexer.buffer);
}
