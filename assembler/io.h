#ifndef __ASM_IO_H
#define __ASM_IO_H

#include "program.h"
#include "lexer.h"
#include "common/io.h"

const char *token_type_to_str(TokenType type);

void print_line_with_underline(const char *filename, Span span, Color color);

void print_span(Span span);
void print_token(Token tok);
void print_program(Program prog);

// ------------------------------------------------------------------------------------------------

void error_unexpected_token(Token unexpected_token, TokenType expected_token_type);
void error_unexpected_token_in_args(Token unexpected_token, size_t types_count, ...);
void error_unexpected_token_in_vec(Token unexpected_token, vector(TokenType) types);
void error_unexpected_comma(Span pos);
void error_missed_label(TokenType instead_of_label, Span pos);
void error_invalid_operand_in_vec(TokenType invalid_op, Span pos, vector(TokenType) valid_types);
void error_unknown_register(const char *reg_name, Span pos);
void error_invalid_character(Span pos);
void error_redefinition(const char *name, Span pos);
void error_invalid_name(const char *name, const char *name_of_what, Span pos);
void error_negative_alignment_size(Span pos);
void error_unresolved_name(Symbol name);
void error_entry_point_with_decls(void);
void error_empty_file(void);
void error_no_entry(void);
void error_unknown_directive(Token directive);
void error_no_input_file(void);

void warning_number_out_of_bounds(long num, long lower_bound, long upper_bound, Span pos);

void note_zero_alignment(Span pos);

#endif
