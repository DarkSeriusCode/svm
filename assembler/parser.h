#ifndef __ASM_PARSER_H
#define __ASM_PARSER_H

#include <stdbool.h>
#include "common/arch.h"
#include "common/vector.h"
#include "lexer.h"

typedef struct {
    Token kind;
    Token value;
    Span span;
} Decl;

Decl new_decl(Token decl_kind, Token decl_value, Span span);
void free_decl(void *decl);

typedef struct {
    const char *name;
    vector(Token) ops;
    Span span;
} Instr;

Instr new_instr(const char *name, vector(Token) ops, Span pos);
void free_instr(void *instr);
void instr_check_ops(Instr instr);
void check_single_op(Token op, size_t expected_types_count, ...);

typedef struct {
    const char *name;
    bool is_empty;
    vector(Token) params;
} Directive;

Directive empty_directive(void);
void directive_set_name(Directive *directive, const char *name);
void directive_check_params(Directive directive);
void free_directive(void *directive);

typedef struct {
    const char *name;
    bool is_data;
    word data_size; // Only if is_data is true
    Span span;
    union {
        vector(Instr) instructions;
        vector(Decl) declarations;
    };
} Label;

Label empty_label(void);
void label_set_name(Label *label, const char *name);
void label_add_instr(Label *label, Instr instr);
void label_add_decl(Label *label, Decl decl);
void free_label(void *label);

// ------------------------------------------------------------------------------------------------

typedef struct {
    const char *filename; // needed for errors
    vector(Token) tokens;
    const char *last_proper_label_name;
    size_t idx;
} Parser;

Parser new_parser(const char *filename);

Token parser_get_checked_token(Parser parser, size_t pos, TokenType tok_type);
Token parser_get_checked_token_in_list(Parser parser, size_t pos, size_t types_count, ...);

Directive parse_directive(Parser *parser);
Label parse_label(Parser *parser);
void parse_declaration(Parser *parser, Label *label);
void parse_instruction(Parser *parser, Label *label);

void free_parser(void *parser);

char *mangle_name(const char *lbl_name, const char *other_name);

#endif
