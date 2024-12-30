#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "io.h"
#include "common/io.h"

const char *token_type_to_str(TokenType type) {
    switch (type) {
        case TOKEN_UNKNOWN:  return "<UNKNOWN>";
        case TOKEN_SECTION:  return "section";
        case TOKEN_IDENT:    return "identifier";
        case TOKEN_COLON:    return "colon";
        case TOKEN_INSTR:    return "instruction";
        case TOKEN_REG:      return "register";
        case TOKEN_COMMA:    return "comma";
        case TOKEN_NUMBER:   return "number";
        case TOKEN_END:      return "end";
        case TOKEN_DECL:     return "declaration";
        case TOKEN_STRING:   return "string";
        case TOKEN_EOF:      return "<EOF>";
        default: return "[undefined]";
    }
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

void print_span(Span span) {
    printf("%lu:%lu", span.line, span.column);
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
