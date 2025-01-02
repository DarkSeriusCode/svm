#include "parser.h"
#include "error.h"
#include "common/arch.h"
#include "common/vector.h"
#include "io.h"
#include <stdio.h>
#include <assert.h>
#include <limits.h>

Parser new_parser(const char *filename) {
    vector(Token) tokens = NULL;
    vector_set_destructor(tokens, free_token);
    Lexer lexer = new_lexer(filename);
    Token tok;
    do {
        tok = lexer_get_next_token(&lexer);
        /* print_token(tok); */
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
    error_unexpected_token_in_vec(tok ,types);
    return new_token(TOKEN_UNKNOWN, "", (Span){0, 0, 0});
}

void parse_code_section(Parser *parser, Image *img) {
    Token tok;
    parser->idx = 0;
    for (;; parser->idx++) {
        tok = parser->tokens[parser->idx + parser->idx];
        if (tok.type == TOKEN_EOF) {
            return;
        }
        if (tok.type == TOKEN_SECTION) {
            Token next_tok = parser->tokens[parser->idx + parser->idx + 1];
            if (next_tok.type == TOKEN_IDENT && strcmp(next_tok.value, "code") == 0) {
                parser->idx += 2;
                break;
            }
        }
    }

    while (parser->tokens[parser->idx].type != TOKEN_END) {
        if (is_label(parser->tokens + parser->idx) != TOKEN_UNKNOWN) {
            error_missed_label(parser->tokens[parser->idx].span);
        }
        Token label_name = parser->tokens[parser->idx];
        image_add_definition(img, label_name.value, vector_size(img->data));
        parser->idx += 2;
        while (is_label(parser->tokens + parser->idx) != TOKEN_UNKNOWN) {
            parse_instruction(parser, img);
            if (parser->tokens[parser->idx].type == TOKEN_END) {
                break;
            }
        }
    }
}

void parse_instruction(Parser *parser, Image *img) {
    Token instr_token = parser->tokens[parser->idx++];
    if (instr_token.type != TOKEN_INSTR) {
        error_unexpected_token(instr_token, TOKEN_INSTR);
    }
    int amount_of_ops = 0;
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
        vector_push_back(ops, tok);
        // Because ops are separeted by commas
        if (i != amount_of_ops - 1) {
            Token comma = parser->tokens[parser->idx++];
            if (comma.type != TOKEN_COMMA) {
                error_unexpected_token(comma, TOKEN_COMMA);
            }
        }
    }
    unsigned long instr = 0;
    size_t instr_size = get_instr_size(instr_token.value);
    instr |= get_instr_opcode(instr_token.value);
    size_t offset = instr_size * 8 - OPCODE_BIT_SIZE;
    instr <<= offset;
    check_instr_ops(instr_token.value, ops);

    for (size_t i = 0; i < vector_size(ops); i++) {
        Token op = ops[i];
        long code;
        char *endptr;
        switch (op.type) {
            case TOKEN_REG:
                code = get_register_code(op.value);
                code <<= (offset -= REGISTER_BIT_SIZE);
                instr |= code;
            break;

            case TOKEN_IDENT:
                offset = (size_t)(offset / 8) * 8;
                NameEntry *ident = image_get_name(*img, op.value);
                if (ident) {
                    code = ident->address;
                    instr |= code;
                    break;
                }
                image_add_declaration(img, op.value,
                                      vector_size(img->data) - 1 + instr_size - offset / 8);
            break;

            case TOKEN_NUMBER:
                offset = (size_t)(offset / 8) * 8;
                // TODO: Show a warning about narrowing convertion
                code = (unsigned short)strtol(op.value, &endptr, 10);
                code <<= (offset - 16);
                instr |= code;
            break;

            default:
                printf("%s is unimplemented yet\n", token_type_to_str(op.type));
        }
    }

    vector(byte) instr_bytes = NULL;
    for (int i = instr_size - 1; i >= 0; i--) {
        vector_push_back(instr_bytes, instr >> i*8);
    }
    image_add_data(img, instr_bytes);
    free_vector(ops);
}

void parse_data_section(Parser *parser, Image *img) {
    Token tok;
    parser->idx = 0;
    for (;; parser->idx++) {
        tok = parser->tokens[parser->idx];
        if (tok.type == TOKEN_EOF) {
            return;
        }
        if (tok.type == TOKEN_SECTION) {
            Token next_tok = parser->tokens[parser->idx + 1];
            if (next_tok.type == TOKEN_IDENT && strcmp(next_tok.value, "data") == 0) {
                parser->idx += 2;
                break;
            }
        }
    }

    while (parser->tokens[parser->idx].type != TOKEN_END) {
        Token decl_name = parser_get_checked_token(*parser, parser->idx++, TOKEN_IDENT);
        Token decl_op = parser_get_checked_token(*parser, parser->idx++, TOKEN_DECL);
        Token decl_val = parser_get_checked_token_in_list(*parser, parser->idx++, 2,
                                                          TOKEN_NUMBER, TOKEN_STRING);
        image_add_definition(img, decl_name.value, vector_size(img->data));
        vector(byte) bytes = NULL;
        if (decl_val.type == TOKEN_NUMBER) {
            char *str_end;
            long val = strtol(decl_val.value, &str_end, 10);
            if (strcmp(decl_op.value, "dw") == 0) {
                vector_push_back(bytes, val >> 8);
            }
            vector_push_back(bytes, val & 0xFF);
        } else {
            for (size_t i = 1; i < strlen(decl_val.value) - 1; i++) {
                vector_push_back(bytes, decl_val.value[i]);
            }
        }
        image_add_data(img, bytes);
    }
}

void free_parser(void *parser) {
    Parser p = *(Parser *)parser;
    free_vector(p.tokens);
}

TokenType is_label(Token *label_ptr) {
    if (label_ptr->type != TOKEN_IDENT) {
        return TOKEN_IDENT;
    }
    if ((label_ptr + 1)->type != TOKEN_COLON) {
        return TOKEN_COLON;
    }
    return TOKEN_UNKNOWN;
}

// ------------------------------------------------------------------------------------------------


void check_instr_ops(const char *instr_name, vector(Token) ops) {
    if (strcmp(instr_name, "load") * strcmp(instr_name, "store") == 0) {
        if (ops[0].type != TOKEN_REG)
            error_invalid_operand(ops[0].type, TOKEN_REG, ops[0].span);
        if (ops[1].type != TOKEN_IDENT)
            error_invalid_operand(ops[1].type, TOKEN_IDENT, ops[1].span);
    }
    if (strcmp(instr_name, "add") * strcmp(instr_name, "sub")
        * strcmp(instr_name, "mul") * strcmp(instr_name, "div") == 0)
    {
        if (ops[0].type != TOKEN_REG)
            error_invalid_operand(ops[0].type, TOKEN_REG, ops[0].span);
        if (ops[1].type != TOKEN_REG)
            error_invalid_operand(ops[1].type, TOKEN_REG, ops[1].span);
    }
    if (strcmp(instr_name, "mov") == 0) {
        if (ops[0].type != TOKEN_REG)
            error_invalid_operand(ops[0].type, TOKEN_REG, ops[0].span);
        if (ops[1].type != TOKEN_REG && ops[1].type != TOKEN_NUMBER)
            error_invalid_operand_in_args(ops[1].type, ops[1].span, 2, TOKEN_NUMBER, TOKEN_REG);
    }
}
