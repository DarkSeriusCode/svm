#ifndef __ASM_ERROR_H
#define __ASM_ERROR_H

#include "lexer.h"

void error_section_not_found(const char *section_name);
void error_unexpected_token(Token unexpected_token, TokenType expected_token_type);
void error_unexpected_token_in_args(Token unexpected_token, size_t types_count, ...);
void error_unexpected_token_in_vec(Token unexpected_token, vector(TokenType) types);
void error_unexpected_comma(Span pos);
void error_missed_label(Span pos);
void error_invalid_operand(TokenType invalid_op, TokenType valid_op, Span pos);
void error_invalid_operand_in_args(TokenType invalid_op, Span pos, size_t valid_types_count, ...);
void error_unknown_register(const char *reg_name, Span pos);
void error_invalid_character(Span pos);

#endif
