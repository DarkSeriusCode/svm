#include "io.h"
#include "error.h"

char *read_whole_file(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        error_file_doesnot_exist(filename);
    }
    // getting len
    fseek(fp, 0, SEEK_END);
    size_t file_len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buffer = malloc(file_len + 1);
    fread(buffer, 1, file_len + 1, fp);
    buffer[file_len] = '\0';

    fclose(fp);
    return buffer;
}

void print_byte(byte num) {
    for (int i = sizeof(num) * 8 - 1; i >= 0; i--) {
        putchar((num & (1U << i)) ? '1' : '0');
    }
}

void print_bitset(BitSet bs) {
    vector(byte) bytes = bitset_to_vec(bs);
    for (size_t i = 0; i < vector_size(bytes); i++) {
        print_byte(bytes[i]);
        printf("  ");
    }
    printf("\n");
    free_vector(bytes);
}

void load_program(byte *memory, size_t memory_size, const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        error_file_doesnot_exist(filename);
    }
    fread(memory, memory_size, sizeof(byte), fp);
    fclose(fp);
}
