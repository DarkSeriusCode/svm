#ifndef __COMMON_IO_H
#define __COMMON_IO_H

#include "assembler/image.h"
#include "assembler/lexer.h"
#include "common/bitset.h"
#include <stdio.h>

// Returns file content. Needs to be freed
char *read_whole_file(const char *filename);

// Prints span.line line from the file, and underline from span.colomn to span.column + span.len
void print_line_with_underline(const char *filename, Span span);

void print_byte(byte num);
void print_span(Span span);
void print_bitset(BitSet bs);
void print_token(Token tok);
void print_image(Image img);

void dump_image(Image image, const char *filename);

#endif
