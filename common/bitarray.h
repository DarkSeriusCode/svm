#ifndef __COMMON_BITSET_H
#define __COMMON_BITSET_H

#include "arch.h"
#include <stdint.h>

typedef struct {
    byte *buffer;
    size_t size;
    size_t capacity;
} BitArray;

BitArray new_bitarray(void);
void bitarray_read_from(BitArray *ba, byte *buffer, size_t bit_count);
bool bitarray_get_bit(BitArray ba, size_t idx);
word bitarray_get_nbits(BitArray ba, size_t n, size_t offset);
void bitarray_push_bit(BitArray *ba, bool bit);
void bitarray_push_nbits(BitArray *ba, size_t n, uint64_t bits);
size_t bitarray_byte_size(BitArray ba);
void free_bitarray(void *bitset);

#endif
