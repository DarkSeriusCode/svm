#ifndef __ASM_ERROR_H
#define __ASM_ERROR_H

#include "lexer.h"
#include "image.h"

void error_section_not_found(const char *section_name);
void error_unexpected_token(Token unexpected_token, TokenType expected_token_type);
void error_unexpected_token_in_args(Token unexpected_token, size_t types_count, ...);
void error_unexpected_token_in_vec(Token unexpected_token, vector(TokenType) types);
void error_unexpected_comma(Span pos);
void error_missed_label(TokenType instead_of_label, Span pos);
void error_invalid_operand(TokenType invalid_op, TokenType valid_op, Span pos);
void error_invalid_operand_in_vec(TokenType invalid_op, Span pos, vector(TokenType) valid_types);
void error_unknown_register(const char *reg_name, Span pos);
void error_invalid_character(Span pos);
void error_redefinition(const char *name, Span pos);
void error_undefined_identifier(Token ident);

void warning_number_out_of_bounds(long num, long lower_bound, long upper_bound, Span pos);
void warning_wrong_ident_size(Token ident, Decl decl, size_t expected_size);

#endif
