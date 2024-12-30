#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "lexer.h"
#include "assembler/error.h"
#include "common/arch.h"
#include "common/io.h"

Token new_token(TokenType type, const char *value, Span span) {
    char *val = NULL;
    if (value != 0) {
        val = malloc(strlen(value) + 1);
        strcpy(val, value);
    }
    return (Token) { type, val, span };
}


void free_token(void *lexem) {
    free((void *)((Token*)lexem)->value);
}

bool is_reg(const char *buffer) {
    if (strcmp(buffer, "sp") * strcmp(buffer, "cf") * strcmp(buffer, "ip") == 0) return true;
    if (buffer[0] != 'r') return false;
    if (buffer[1] != '\0' && is_number(buffer + 1)) return true;
    return false;
}

bool is_ident(const char *buffer) {
    if (isdigit(buffer[0])) return false;
    for (size_t i = 0; i < strlen(buffer); i++)
        if (!(buffer[i] == '_' || isalnum(buffer[i]))) return false;
    return true;
}

bool is_number(const char *buffer) {
    for (size_t i = 0; i < strlen(buffer); i++) {
        if (buffer[i] == '-') continue;
        if (!isdigit(buffer[i])) return false;
    }
    return true;
}

const char *is_incorrect(const char *buffer) {
    for (const char *c = buffer; *c != '\0'; c++)
        if (*c != '-' && *c != '_' && !isalnum(*c)) return c;
    return NULL;
}

// ------------------------------------------------------------------------------------------------

static Span calc_span(Span span, const char *buffer) {
    return (Span) { span.column - strlen(buffer), span.line, strlen(buffer) };
}

Lexer new_lexer(const char *file_name) {
    vector(Token) token_buffer = NULL;
    vector_reserve(token_buffer, 3);

    return (Lexer){
        .i = 0,
        .c = 0,
        .source_file = read_whole_file(file_name),
        .current_span = (Span){ 1, 1, 0 },
        .file_name = file_name,
        .token_buffer = token_buffer,
    };
}

void lexer_advice(Lexer *lexer) {
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
    size_t start_pos = lexer->i;
    lexer_advice(lexer);
    while (lexer->c != '"') lexer_advice(lexer);
    size_t len = lexer->i - start_pos + 1;

    char string[len+1];
    memset(string, 0, len);
    strncpy(string, lexer->source_file + start_pos - 1, len);
    string[len] = '\0';

    return new_token(TOKEN_STRING, string, calc_span(lexer->current_span, string));
}

void lexer_skip_whitespaces(Lexer *lexer) {
    if (lexer->i >= strlen(lexer->source_file)) return;
    while (isspace(lexer->c)) lexer_advice(lexer);
}

void lexer_skip_comment(Lexer *lexer) {
    while (lexer->c == ';') {
        while (lexer->c != 10) lexer_advice(lexer);
        lexer_advice(lexer);
    }
}

Token make_nonterm(Lexer *lexer, const char *buffer, Span token_span) {
    if (is_reg(buffer)) {
        if (!in_register_set(buffer))
            error_unknown_register(buffer, token_span);
        return new_token(TOKEN_REG, buffer, token_span);
    }
    if (is_ident(buffer))
        return new_token(TOKEN_IDENT, buffer, token_span);
    if (is_number(buffer))
        return new_token(TOKEN_NUMBER, buffer, token_span);
    return new_token(TOKEN_UNKNOWN, buffer, lexer->current_span);
}

Token lexer_get_next_token(Lexer *lexer) {
    Span tok_span;
    char buffer[1024];
    size_t i = 0;
    if (!vector_empty(lexer->token_buffer)) {
        Token tok = lexer->token_buffer[0];
        vector_erase(lexer->token_buffer, 0);
        return tok;
    }
    memset(buffer, 0, sizeof(buffer));

    lexer_advice(lexer);
    do {
        // if the entire line is a comment it will skip it.
        lexer_skip_comment(lexer);
        lexer_skip_whitespaces(lexer);
        if (lexer->i >= strlen(lexer->source_file))
            return new_token(TOKEN_EOF, "", lexer->current_span);
    } while (isspace(lexer->c) || lexer->c == ';');

    while (!isspace(lexer->c)) {
        switch (lexer->c) {
            case '"': return lex_string(lexer);
            case ';': lexer_skip_comment(lexer); continue;
            case ',': {
                Token comma = new_token(TOKEN_COMMA, ",", calc_span(lexer->current_span, ","));
                vector_push_back(lexer->token_buffer, comma);
                if (strlen(buffer)) {
                    return make_nonterm(lexer, buffer, tok_span);
                }
                return lexer_get_next_token(lexer);
            }; break;
            case ':': {
                Token colon = new_token(TOKEN_COLON, ":", calc_span(lexer->current_span, ":"));
                vector_push_back(lexer->token_buffer, colon);
                if (strlen(buffer)) {
                    return make_nonterm(lexer, buffer, tok_span);
                }
                return lexer_get_next_token(lexer);
            } break;
        }
        buffer[i++] = tolower(lexer->c);
        tok_span = calc_span(lexer->current_span, buffer);

        if (strcmp(buffer, "section") == 0)
            return new_token(TOKEN_SECTION, buffer, tok_span);
        if (strcmp(buffer, "end") == 0)
            return new_token(TOKEN_END, buffer, tok_span);
        if (strcmp(buffer, "db") == 0 || strcmp(buffer, "dw") == 0)
            return new_token(TOKEN_DECL, buffer, tok_span);
        if (in_instruction_set(buffer))
            return new_token(TOKEN_INSTR, buffer, tok_span);
        const char *incorrect_at = is_incorrect(buffer);
        if (incorrect_at != NULL) {
            tok_span.column += tok_span.len - 1;
            tok_span.len = 1;
            error_invalid_character(tok_span);
        }
        lexer_advice(lexer);
    }
    assert(strlen(buffer) != 0);
    return make_nonterm(lexer, buffer, tok_span);
}

void free_lexer(void *lexer_ptr) {
    Lexer lexer = *(Lexer *)lexer_ptr;
    free((void *)lexer.source_file);
    free_vector(lexer.token_buffer);
}
