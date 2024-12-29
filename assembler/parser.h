#ifndef __ASM_PARSER_H
#define __ASM_PARSER_H

#include <common/vector.h>
#include "assembler/image.h"
#include "common/bitset.h"
#include "lexer.h"
#include <stdint.h>

typedef struct {
    const char *filename; // needed for errors
    vector(Token) tokens;
    size_t idx;
} Parser;

Parser new_parser(const char *filename);

Token parser_get_checked_token(Parser parser, size_t pos, TokenType tok_type);
Token parser_get_checked_token_in_list(Parser parser, size_t pos, size_t types_count, ...);

void parse_instrution(Parser *parser, Image *img);

void parse_code_section(Parser *parser, Image *img);
void parse_data_section(Parser *parser, Image *img);

void free_parser(void *parser);

// Checks if the next two tokens at `label_ptr` are parts of label token.
// Returns TOKEN_UNKNOWN if there's a label, otherwise returns type of missed token
TokenType is_label(Token *label_ptr);

void check_instr_ops(const char *instr_name, vector(Token) ops);

#endif
