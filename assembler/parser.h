#ifndef __ASM_PARSER_H
#define __ASM_PARSER_H

#include "common/vector.h"
#include "lexer.h"

typedef struct {
    Token kind;
    Token value;
    Span span;
} Decl;

Decl new_decl(Token decl_kind, Token decl_value, Span span);

typedef struct {
    const char *name;
    vector(Token) ops;
    Span span;
} Instr;

Instr new_instr(const char *name, vector(Token) ops, Span pos);
void instr_check_ops(Instr instr);
void check_single_op(Token op, size_t expected_types_count, ...);

typedef struct {
    const char *name;
    bool is_data;
    bool is_empty;
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
    size_t idx;
} Parser;

Parser new_parser(const char *filename);

Token parser_get_checked_token(Parser parser, size_t pos, TokenType tok_type);
Token parser_get_checked_token_in_list(Parser parser, size_t pos, size_t types_count, ...);

Label parse_label(Parser *parser);
void parse_declaration(Parser *parser, Label *label);
void parse_instruction(Parser *parser, Label *label);

void free_parser(void *parser);

#endif
