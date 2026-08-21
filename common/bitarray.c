#include <stdlib.h>
#include <string.h>
#include "bitarray.h"
#include <assert.h>
#include "utils.h"
#include <stdio.h>

BitArray new_bitarray() {
    size_t byte_count = 1;

    byte *buffer = malloc(byte_count);
    memset(buffer, 0, byte_count);

    return (BitArray) { buffer, 0, 0 };
}

void bitarray_read_from(BitArray *ba, byte *buffer, size_t bit_count) {
    size_t i = 0;
    while (bit_count > 0) {
        size_t bits_to_read = min(bit_count, 8);
        byte b = buffer[i++];
        for (size_t j = 0; j < bits_to_read; j++) {
            byte temp = b & (1 << (7 - j));
            bitarray_push_bit(ba, temp > 0);
        }
        bit_count -= bits_to_read;
    }
}

bool bitarray_get_bit(BitArray ba, size_t idx) {
    assert(ba.size >= idx);
    size_t byte_idx = idx / 8;
    return ba.buffer[byte_idx] & (1 << (7 - idx % 8));
}

word bitarray_get_nbits(BitArray ba, size_t n, size_t offset) {
    word value = 0;
    for (size_t i = offset; i < n + offset; i++) {
        value <<= 1;
        value |= bitarray_get_bit(ba, i);
    }
    return value;
}

void bitarray_push_bit(BitArray *ba, bool bit) {
    if (ba->size >= ba->capacity || ba->capacity == 0) {
        size_t new_byte_cap = bitarray_byte_size(*ba) + 1;
        ba->buffer = realloc(ba->buffer, new_byte_cap);
        memset(ba->buffer + new_byte_cap - 1, 0, 8);
        ba->capacity = new_byte_cap * 8;
    }
    size_t byte_idx = ba->size / 8;
    ba->buffer[byte_idx] ^= bit << (7 - (ba->size % 8));
    ba->size++;
}

void bitarray_push_nbits(BitArray *ba, size_t n, uint64_t bits) {
    for (int i = n - 1; i >= 0; i--) {
        bitarray_push_bit(ba, (bits >> i) & 1);
    }
}

size_t bitarray_byte_size(BitArray ba) {
    // TODO: Move that in the separate function
    return ba.size / 8 + ((ba.size % 8 != 0) ? 1 : 0);
}

void free_bitarray(void *bitset) {
    free((void*)((BitArray*)bitset)->buffer);
}
