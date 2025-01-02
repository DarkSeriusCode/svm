#ifndef __COMMON_IO_H
#define __COMMON_IO_H

#include "bitset.h"
#include <stdio.h>

// Returns file content. Needs to be freed
char *read_whole_file(const char *filename);

void print_byte(byte num);
void print_bitset(BitSet bs);

// Returns the size of the loaded program
size_t load_program(byte *memory, size_t memory_size, const char *filename);

#endif
