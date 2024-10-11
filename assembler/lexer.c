#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "lexer.h"
#include "common/arch.h"
#include "common/io.h"

Token new_token(TokenType type, const char *value, Span span) {
    char *val = malloc(strlen(value));
    strcpy(val, value);
    return (Token) { type, val, span };
}

const char *token_type_to_str(TokenType type) {
    switch (type) {
        case TOKEN_UNKNOWN: return "<UNKNOWN>";
        case TOKEN_SECTION: return "<SECTION>";
        case TOKEN_IDENT:   return "<IDENT>";
        case TOKEN_LABEL:   return "<LABEL>";
        case TOKEN_INSTR:   return "<INSTR>";
        case TOKEN_REG:     return "<REG>";
        case TOKEN_COMMA:   return "<COMMA>";
        case TOKEN_NUMBER:  return "<NUMBER>";
        case TOKEN_END:     return "<END>";
        case TOKEN_DECL:    return "<DECL>";
        case TOKEN_STRING:  return "<STRING>";
        case TOKEN_EOF:     return "<EOF>";
        default: return "[undefined]";
    }
}

void free_token(Token lexem) {
    free((void*)lexem.value);
}

bool is_ident(const char *buffer) {
    if (isdigit(buffer[0])) return false;
    for (size_t i = 0; i < strlen(buffer); i++)
        if (!(buffer[i] == '_' || isalnum(buffer[i]))) return false;
    return true;
}

bool is_label(const char *buffer) {
    char substr[strlen(buffer)];
    strncpy(substr, buffer, sizeof(substr) - 1);
    if (is_ident(substr) && buffer[strlen(buffer) - 1] == ':') return true;
    return false;
}

bool is_instr(const char *buffer) {
    for (size_t i = 0; i < sizeof(INSTRUCTION_SET)/sizeof(INSTRUCTION_SET[0]); i++)
        if (strcmp(INSTRUCTION_SET[i], buffer) == 0) return true;
    return false;
}

// TODO: Refactor `is_reg` and `is_instr`. Make it a single function like `contains` or smth
bool is_reg(const char *buffer) {
    for (size_t i = 0; i < sizeof(REGISTER_SET)/sizeof(REGISTER_SET[0]); i++)
        if (strcmp(REGISTER_SET[i], buffer) == 0) return true;
    return false;
}

bool is_number(const char *buffer) {
    for (size_t i = 0; i < strlen(buffer); i++)
        if (!isdigit(buffer[i])) return false;
    return true;
}

// ------------------------------------------------------------------------------------------------

static Span calc_span(Span span, const char *buffer) {
    return (Span) { span.column - strlen(buffer), span.line };
}

Lexer new_lexer(const char *file_name) {
    return (Lexer){
        .i = 0,
        .c = 0,
        .source_file = read_whole_file(file_name),
        .current_span = (Span){ 1, 1, },
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

    size_t len = lexer->i - start_pos - 1;

    char string[len+1];
    memset(string, 0, len);
    strncpy(string, lexer->source_file + start_pos, len);
    string[len] = '\0';

    return new_token(TOKEN_STRING, string, calc_span(lexer->current_span, string));
}

void lexer_skip_whitespaces(Lexer *lexer) {
    if (lexer->i >= strlen(lexer->source_file)) return;
    while (isspace(lexer->c)) lexer_advice(lexer);
}

void lexer_skip_comment(Lexer *lexer) {
    while (lexer->c != 10) lexer_advice(lexer);
    lexer_advice(lexer);
}

Token lexer_get_next_token(Lexer *lexer) {
    Span tok_span;
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    lexer_advice(lexer);
    lexer_skip_whitespaces(lexer);
    if (lexer->i >= strlen(lexer->source_file))
        return new_token(TOKEN_EOF, "", lexer->current_span);
    for (size_t i = 0; !isspace(lexer->c); i++, lexer_advice(lexer)) {
        switch (lexer->c) {
            case ',': return new_token(TOKEN_COMMA, ",", calc_span(lexer->current_span, ","));
            case '"': return lex_string(lexer);
            case ';': lexer_skip_comment(lexer);
        }
        buffer[i] = lexer->c;
        // NOTE: in case of "<ident>," lexer's gonna segfault
        // because it thinks that <ident> and comma are one token

        tok_span = calc_span(lexer->current_span, buffer);

        if (strcmp(buffer, "section") == 0)
            return new_token(TOKEN_SECTION, buffer, tok_span);
        if (strcmp(buffer, "end") == 0)
            return new_token(TOKEN_END, buffer, tok_span);
        if (strcmp(buffer, "db") == 0 || strcmp(buffer, "dw") == 0)
            return new_token(TOKEN_DECL, buffer, tok_span);
        if (is_instr(buffer))
            return new_token(TOKEN_INSTR, buffer, tok_span);
        if (is_reg(buffer))
            return new_token(TOKEN_REG, buffer, tok_span);
    }
    if (strlen(buffer) == 0) {
        exit_with_erorr("Something went wrong and i have actually no idea why. Don't ask me ;-;");
    }
    if (is_label(buffer))
        return new_token(TOKEN_LABEL, buffer, tok_span);
    if (is_ident(buffer))
        return new_token(TOKEN_IDENT, buffer, tok_span);
    if (is_number(buffer))
        return new_token(TOKEN_NUMBER, buffer, tok_span);

    return new_token(TOKEN_UNKNOWN, 0, lexer->current_span);
}

void free_lexer(Lexer lexer) {
    free((void*)lexer.source_file);
}
