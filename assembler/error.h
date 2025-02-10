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
void error_missed_bracket(const char *bracket, Span pos);
void error_invalid_name(const char *name, const char *name_of_what, Span span);
void error_negative_alignment_size(Span pos);
void error_unresolved_name(Symbol name);

void warning_number_out_of_bounds(long num, long lower_bound, long upper_bound, Span pos);
void warning_empty_label(Label lbl);

#endif
