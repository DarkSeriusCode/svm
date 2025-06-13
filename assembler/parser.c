#include "parser.h"
#include "common/utils.h"
#include "io.h"
#include "common/arch.h"
#include "common/vector.h"
#include <stdio.h>
#include <assert.h>

Decl new_decl(Token decl_kind, Token decl_value, Span span) {
    return (Decl){ decl_kind, decl_value, span };
}

void free_decl(void *decl) {
    Decl *d = (Decl *)decl;
    free_token(&d->kind);
    free_token(&d->value);
}

Instr new_instr(const char *name, vector(Token) ops, Span pos) {
    char *allocated_str = malloc(strlen(name) + 1);
    strcpy(allocated_str, name);
    return (Instr){ allocated_str, ops, pos };
}

void free_instr(void *instr) {
    Instr *i = (Instr *)instr;
    free_vector(&i->ops);
}

void check_single_op(Token op, size_t expected_types_count, ...) {
    va_list args;
    va_start(args, expected_types_count);
    vector(TokenType) types = NULL;
    for (size_t i = 0; i < expected_types_count; i++) {
        TokenType tt = va_arg(args, TokenType);
        vector_push_back(types, tt);
        if (op.type == tt) {
            return;
        }
    }
    if (expected_types_count == 1 && types[0] == TOKEN_REG) {
        error_unknown_register(op.value, op.span);
    }
    if (op.type == TOKEN_UNKNOWN) {
        error_invalid_name(op.value, "identifier", op.span);
    }
    error_invalid_operand_in_vec(op.type, op.span, types);
    va_end(args);
}

void instr_check_ops(Instr instr) {
    const char *instr_name = instr.name;
    vector(Token) ops = instr.ops;

    if (string_in_args(instr_name, 4, "mov", "load", "store", "cmp")) {
        check_single_op(ops[0], 1, TOKEN_REG);
        check_single_op(ops[1], 3, TOKEN_REG, TOKEN_NUMBER, TOKEN_IDENT);
    }
    if (string_in_args(instr_name, 9, "add", "sub", "mul", "div", "and", "or", "xor", "shl", "shr"))
    {
        check_single_op(ops[0], 1, TOKEN_REG);
        check_single_op(ops[1], 3, TOKEN_REG, TOKEN_NUMBER, TOKEN_IDENT);
    }
    if (string_in_args(instr_name, 3, "not", "push", "pop")) {
        check_single_op(ops[0], 1, TOKEN_REG);
    }
    if (string_in_args(instr_name, 2, "call", "jmp")) {
        check_single_op(ops[0], 1, TOKEN_IDENT);
    }
    if (strcmp(instr_name, "jif") == 0) {
        check_single_op(ops[0], 1, TOKEN_CMP);
        check_single_op(ops[1], 1, TOKEN_IDENT);
    }
    if (string_in_args(instr_name, 2, "out", "in")) {
        check_single_op(ops[0], 1, TOKEN_NUMBER);
        check_single_op(ops[1], 3, TOKEN_NUMBER, TOKEN_IDENT, TOKEN_REG);
        check_single_op(ops[2], 3, TOKEN_NUMBER, TOKEN_IDENT, TOKEN_REG);
    }
    // ret instruction doesn't need a check (it has no params ;-;)
}

Directive empty_directive(void) {
    return (Directive) {
        .is_empty = true,
    };
}

void directive_set_name(Directive *directive, const char *name) {
    char *buffer = malloc(strlen(name) + 1);
    strcpy(buffer, name);
    if (directive->name != NULL) {
        free((void *)directive->name);
    }
    directive->name = buffer;
}

void directive_check_params(Directive directive) {
    vector(Token) ops = directive.params;

    if (strcmp(directive.name, "use") == 0) {
        check_single_op(ops[0], 1, TOKEN_STRING);
        check_single_op(ops[1], 1, TOKEN_NUMBER);
    }
}

void free_directive(void *directive) {
    Directive *dir = (Directive *)directive;
    if (!dir->is_empty) {
        free((void *)dir->name);
        free_vector(&dir->params);
    }
}

Label empty_label(void) {
    return (Label){ NULL, false, 0, (Span){0, 0, 0}, {NULL} };
}

void label_set_name(Label *label, const char *name) {
    char *allocated_str = malloc(strlen(name) + 1);
    strcpy(allocated_str, name);
    label->name = allocated_str;
}

void label_add_instr(Label *label, Instr instr) {
    vector_push_back(label->instructions, instr);
}

void label_add_decl(Label *label, Decl decl) {
    vector_push_back(label->declarations, decl);
}

void free_label(void *label) {
    Label lbl = *(Label *)label;
    if (lbl.name)
        free((void *)lbl.name);
    if (lbl.is_data) {
        vector_set_destructor(lbl.declarations, free_decl);
        free_vector(&lbl.declarations);
    } else {
        vector_set_destructor(lbl.instructions, free_instr);
        free_vector(&lbl.instructions);
    }
}

// ------------------------------------------------------------------------------------------------

Parser new_parser(const char *filename) {
    vector(Token) tokens = NULL;
    vector_set_destructor(tokens, free_token);
    Lexer lexer = new_lexer(filename);
    Token tok;
    do {
        tok = lexer_get_next_token(&lexer);
        vector_push_back(tokens, tok);
    } while (tok.type != TOKEN_EOF && tok.type != TOKEN_UNKNOWN);
    free_lexer(&lexer);

    return (Parser) {
        .tokens = tokens,
        .idx = 0,
        .filename = filename,
        .last_proper_label_name = NULL,
    };
}

Token parser_get_checked_token(Parser parser, size_t pos, TokenType tok_type) {
    Token tok = parser.tokens[pos];
    if (tok.type == TOKEN_EOF) {
        // Just made it for pretty error messages
        tok = parser.tokens[pos - 1];
        tok.span.column += tok.span.column + 1;
        tok.span.len = 3;
        error_unexpected_token(tok, tok_type);
    }
    if ((tok.type == TOKEN_IDENT || tok.type == TOKEN_INSTR) && tok_type == TOKEN_LABEL) {
        error_missed_label(tok.type, tok.span);
    }
    if (tok.type != tok_type) {
        error_unexpected_token(tok, tok_type);
    }
    return tok;
}

Token parser_get_checked_token_in_list(Parser parser, size_t pos, size_t types_count, ...) {
    va_list args;
    va_start(args, types_count);
    Token tok = parser.tokens[pos];
    vector(TokenType) types = NULL;

    for (size_t i = 0; i < types_count; i++) {
        TokenType t = va_arg(args, TokenType);
        if (tok.type == t) {
            va_end(args);
            free_vector(&types);
            return tok;
        }
        vector_push_back(types, t);
    }
    va_end(args);
    error_unexpected_token_in_vec(tok, types);
    return new_token(TOKEN_UNKNOWN, "", (Span){0, 0, 0});
}

Directive parse_directive(Parser *parser) {
    Directive directive = empty_directive();
    Token dir_tok = parser_get_checked_token(*parser, parser->idx++, TOKEN_DIRECTIVE);
    size_t amount_of_params = get_dir_param_count(dir_tok.value);
    vector(Token) params = NULL;
    vector_set_destructor(params, free_token);
    for (size_t i = 0; i < amount_of_params; i++) {
        Token tok = parser->tokens[parser->idx++];
        vector_push_back(params, copy_token(tok));
    }
    directive_set_name(&directive, dir_tok.value);
    directive.params = params;
    directive.is_empty = false;
    directive_check_params(directive);
    return directive;
}

Label parse_label(Parser *parser) {
    Label label = empty_label();
    Token lbl_tok = parser_get_checked_token(*parser, parser->idx++, TOKEN_LABEL);
    const char *label_name = lbl_tok.value;
    char *new_name = NULL;
    if (parser->last_proper_label_name && lbl_tok.value[0] == '.') {
        new_name = mangle_name(parser->last_proper_label_name, lbl_tok.value);
        label_name = new_name;
    }
    label_set_name(&label, label_name);
    if (new_name) free((void *)new_name);
    if (lbl_tok.value[0] != '.') parser->last_proper_label_name = lbl_tok.value;

    label.span = lbl_tok.span;
    if (parser->tokens[parser->idx].type == TOKEN_LABEL) {
        return label;
    }
    Token tok = parser_get_checked_token_in_list(*parser, parser->idx, 2, TOKEN_DECL, TOKEN_INSTR);
    if (tok.type != TOKEN_INSTR && tok.type != TOKEN_DECL) {
        return label;
    }
    void (*parser_func)(Parser *, Label *) = &parse_instruction;

    if (tok.type == TOKEN_DECL) {
        label.is_data = true;
        parser_func = &parse_declaration;
    }

    do {
        parser_func(parser, &label);
        tok = parser->tokens[parser->idx];
    } while (tok.type != TOKEN_LABEL && tok.type != TOKEN_EOF);

    return label;
}

void parse_declaration(Parser *parser, Label *label) {
    if (parser->tokens[parser->idx].type == TOKEN_EOF) return;
    Token kind = parser_get_checked_token(*parser, parser->idx++, TOKEN_DECL);
    Token value;
    size_t size = 0;
    if (strcmp(kind.value, ".sizeof") == 0) {
        value = parser_get_checked_token(*parser, parser->idx++, TOKEN_IDENT);
        size += 2;
    } else if (strcmp(kind.value, ".ascii") == 0) {
        value = parser_get_checked_token(*parser, parser->idx++, TOKEN_STRING);
        size += strlen(value.value);
    } else {
        if (strcmp(kind.value, ".byte") == 0) size += 1;
        if (strcmp(kind.value, ".word") == 0) size += 2;
        value = parser_get_checked_token(*parser, parser->idx++, TOKEN_NUMBER);
    }
    Span decl_pos = kind.span;
    decl_pos.len = value.span.column - kind.span.column + value.span.len;
    Decl decl = new_decl(copy_token(kind), copy_token(value), decl_pos);
    label_add_decl(label, decl);
    label->data_size += size;
}

void parse_instruction(Parser *parser, Label *label) {
    Token instr_token = parser_get_checked_token(*parser, parser->idx++, TOKEN_INSTR);
    int amount_of_ops = 0;
    if (in_one_op_instruction_set(instr_token.value)) {
        amount_of_ops = 1;
    }
    if (in_two_ops_instruction_set(instr_token.value)) {
        amount_of_ops = 2;
    }
    if (in_three_ops_instruction_set(instr_token.value)) {
        amount_of_ops = 3;
    }
    vector(Token) ops = NULL;
    for (int i = 0; i < amount_of_ops; i++) {
        Token tok = parser->tokens[parser->idx++];
        if (tok.type == TOKEN_COMMA) {
            error_unexpected_comma(tok.span);
        }
        if (tok.value[0] == '.') {
            char *new_name = mangle_name(parser->last_proper_label_name, tok.value);
            tok.value = new_name;
        }
        vector_push_back(ops, copy_token(tok));
        // Because ops are separeted by commas
        if (i != amount_of_ops - 1) {
            Token comma = parser->tokens[parser->idx++];
            if (comma.type != TOKEN_COMMA) {
                error_unexpected_token(comma, TOKEN_COMMA);
            }
        }
    }
    Span instr_pos = instr_token.span;
    if (amount_of_ops != 0) {
        instr_pos.len = ops[amount_of_ops].span.column - instr_token.span.column
                        + ops[amount_of_ops].span.len;
    }
    Instr instr = new_instr(instr_token.value, ops, instr_pos);
    instr_check_ops(instr);
    label_add_instr(label, instr);
}

void free_parser(void *parser) {
    Parser p = *(Parser *)parser;
    free_vector(&p.tokens);
}

char *mangle_name(const char *lbl_name, const char *other_name) {
    char *new_name = malloc(strlen(lbl_name) + strlen(other_name));
    sprintf(new_name, "%s%s", lbl_name, other_name);
    return new_name;
}
