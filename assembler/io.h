#ifndef __ASM_IO_H
#define __ASM_IO_H

#include "assembler/image.h"
#include "assembler/lexer.h"

const char *token_type_to_str(TokenType type);

void print_line_with_underline(const char *filename, Span span);

void print_span(Span span);
void print_token(Token tok);
void print_image(Image img);

void dump_image(Image image, const char *filename);

#endif
