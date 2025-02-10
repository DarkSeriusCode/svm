#ifndef __ASM_IO_H
#define __ASM_IO_H

#include "image.h"
#include "lexer.h"
#include "parser.h"

#define max(a, b) ((a) > (b) ? (a) : (b))

const char *token_type_to_str(TokenType type);

void print_line_with_underline(const char *filename, Span span);

void print_span(Span span);
void print_token(Token tok);
void print_image(Image img);
void print_instr(Instr instr);
void print_decl(Decl decl);
void print_label(Label lbl);

void dump_image(Image image, const char *filename);

#endif
