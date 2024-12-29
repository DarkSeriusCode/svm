#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "io.h"
#include "common/error.h"

char *read_whole_file(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        throw_error(FILE_DOESNOT_EXIST, filename);
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

void print_line_with_underline(const char *filename, Span span) {
    char *file_content = read_whole_file(filename);
    size_t new_lines = 0;
    while (new_lines < span.line - 3 && *file_content != '\0') {
        if (*file_content == 10) {
            new_lines++;
        }
        file_content++;
    }
    char *line = strtok(file_content, "\n");
    char offset[8];
    for (size_t i = 0; i < 3; i++) {
        sprintf(offset, "%lu | ", span.line - 2 + i);
        printf("%s%s\n", offset, line);

        line = strtok(NULL, "\n");
    }
    for (size_t i = 0; i < span.column - 1 + strlen(offset); i++) {
        printf(" ");
    }
    for (size_t i = 0; i < span.len; i++) {
        printf("~");
    }
    printf("\n");
}


void print_byte(byte num) {
    for (int i = sizeof(num) * 8 - 1; i >= 0; i--) {
        putchar((num & (1U << i)) ? '1' : '0');
    }
}

void print_span(Span span) {
    printf("%lu:%lu", span.line, span.column);
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

void print_token(Token tok) {
    printf("%s: `%s` at (%lu, %lu)\n", token_type_to_str(tok.type), tok.value,
                                     tok.span.column, tok.span.line);
}

void print_image(Image img) {
    printf("Symbol tabel:\n");
    size_t longest_name_len = 0;
    for (size_t i = 0; i < vector_size(img.sym_table); i++) {
        size_t name_len = strlen(img.sym_table[i].name);
        if (longest_name_len < name_len) {
            longest_name_len = name_len;
        }
    }
    for (size_t i = 0; i < vector_size(img.sym_table); i++) {
        NameEntry entry = img.sym_table[i];
        printf("%s", entry.name);
        for (size_t j = strlen(entry.name); j < longest_name_len; j++) {
            printf(".");
        }
        printf(":    0x%04x %s", entry.address, (entry.is_resolved) ? "" : "(unresolved)");
        printf("\n");
    }

    printf("\nHex:\n");
    for (size_t i = 0; i < vector_size(img.data); i++) {
        if (i > 0 && i % 16 == 0) {
            printf("\n");
        }
        printf("%02x ", img.data[i]);
    }
    printf("\n");
}

void dump_image(Image image, const char *filename) {
    FILE *fp = fopen(filename, "wb");
    fwrite(image.data, vector_size(image.data), sizeof(byte), fp);
    fclose(fp);
}
