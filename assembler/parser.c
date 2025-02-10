#include "parser.h"
#include "error.h"
#include "common/arch.h"
#include "common/vector.h"
#include <stdio.h>
#include <assert.h>
#include <limits.h>

Decl new_decl(Token decl_kind, Token decl_value, Span span) {
    return (Decl){ decl_kind, decl_value, span };
}

Instr new_instr(const char *name, vector(Token) ops, Span pos) {
    char *allocated_str = malloc(strlen(name) + 1);
    strcpy(allocated_str, name);
    vector(Token) new_ops = NULL;
    for (size_t i = 0; i < vector_size(ops); i++) {
        vector_push_back(new_ops, copy_token(ops[i]));
    }
    return (Instr){ allocated_str, new_ops, pos };
}

void check_number_bounds(Token op, size_t should_has_size) {
    assert(should_has_size == 1 || should_has_size == 2);
    char *strend;
    long n = strtol(op.value, &strend, 10);
    long lower_bound = 0, upper_bound = 0;
    if (should_has_size == 1) {
        if (n < 0) {
            lower_bound = CHAR_MIN;
            upper_bound = CHAR_MAX;
        } else {
            lower_bound = 0;
            upper_bound = UCHAR_MAX;
        }
    } else {
        if (n < 0) {
            lower_bound = SHRT_MIN;
            upper_bound = SHRT_MAX;
        } else {
            lower_bound = 0;
            upper_bound = USHRT_MAX;
        }
    }
    if (!(lower_bound < n && n < upper_bound)) {
        warning_number_out_of_bounds(n, lower_bound, upper_bound, op.span);
    }
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
    error_invalid_operand_in_vec(op.type, op.span, types);
    va_end(args);
}

void instr_check(Instr instr) {
    const char *instr_name = instr.name;
    vector(Token) ops = instr.ops;

    if (strcmp(instr_name, "mov") == 0) {
        check_single_op(ops[0], 1, TOKEN_REG);
        check_single_op(ops[1], 1, TOKEN_REG);
    }
    if (strcmp(instr_name, "load") * strcmp(instr_name, "store") == 0) {
        check_single_op(ops[0], 1, TOKEN_REG);
        check_single_op(ops[1], 3, TOKEN_IDENT, TOKEN_REG, TOKEN_NUMBER);
    }
    if (strcmp(instr_name, "add") * strcmp(instr_name, "sub")
        * strcmp(instr_name, "mul") * strcmp(instr_name, "div") == 0)
    {
        check_single_op(ops[0], 1, TOKEN_REG);
        check_single_op(ops[1], 1, TOKEN_REG);
    }
    if (strcmp(instr_name, "movi") == 0) {
        check_single_op(ops[0], 1, TOKEN_REG);
        check_single_op(ops[1], 2, TOKEN_NUMBER, TOKEN_IDENT);
        if (ops[1].type == TOKEN_NUMBER) {
            check_number_bounds(ops[1], 2);
        }
    }
}

Label empty_label(void) {
    return (Label){ NULL, false, false, (Span){0, 0, 0}, {NULL} };
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
    Label *lbl = (Label *)label;
    if (lbl->name)
        free((void *)lbl->name);
    if (lbl->is_data) {
        free_vector(lbl->declarations);
    } else {
        free_vector(lbl->instructions);
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
    } while (tok.type != TOKEN_EOF);
    free_lexer(&lexer);

    return (Parser) {
        .tokens = tokens,
        .idx = 0,
        .filename = filename,
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
            free_vector(types);
            return tok;
        }
        vector_push_back(types, t);
    }
    va_end(args);
    error_unexpected_token_in_vec(tok, types);
    return new_token(TOKEN_UNKNOWN, "", (Span){0, 0, 0});
}

Label parse_label(Parser *parser) {
    Label label = empty_label();
    if (parser->tokens[parser->idx].type == TOKEN_EOF) {
        return label;
    }
    Token lbl_tok = parser_get_checked_token(*parser, parser->idx++, TOKEN_LABEL);
    label_set_name(&label, lbl_tok.value);
    label.span = lbl_tok.span;
    if (parser->tokens[parser->idx].type == TOKEN_LABEL) {
         return label;
    }
    Token tok = parser->tokens[parser->idx];
    if (tok.type != TOKEN_INSTR && tok.type != TOKEN_DECL) {
        label.is_empty = true;
        return label;
    }
    label.is_empty = false;
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
    if (strcmp(kind.value, ".ascii") == 0) {
        value = parser_get_checked_token(*parser, parser->idx++, TOKEN_STRING);
    } else {
        value = parser_get_checked_token(*parser, parser->idx++, TOKEN_NUMBER);
    }
    Span decl_pos = kind.span;
    decl_pos.len = value.span.column - kind.span.column + value.span.len;
    Decl decl = new_decl(kind, value, decl_pos);
    label_add_decl(label, decl);
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
    assert(amount_of_ops != 0);
    vector(Token) ops = NULL;
    for (int i = 0; i < amount_of_ops; i++) {
        Token tok = parser->tokens[parser->idx++];
        if (tok.type == TOKEN_COMMA) {
            error_unexpected_comma(tok.span);
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
    instr_pos.len = ops[amount_of_ops].span.column - instr_token.span.column
                    + ops[amount_of_ops].span.len;
    Instr instr = new_instr(instr_token.value, ops, instr_pos);
    instr_check(instr);
    label_add_instr(label, instr);
}

void free_parser(void *parser) {
    Parser p = *(Parser *)parser;
    free_vector(p.tokens);
}
