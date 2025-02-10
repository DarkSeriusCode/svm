#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "io.h"
#include "common/io.h"

const char *token_type_to_str(TokenType type) {
    switch (type) {
        case TOKEN_UNKNOWN:     return "<UNKNOWN>";
        case TOKEN_IDENT:       return "identifier";
        case TOKEN_LABEL:       return "label";
        case TOKEN_INSTR:       return "instruction";
        case TOKEN_REG:         return "register";
        case TOKEN_COMMA:       return "comma";
        case TOKEN_NUMBER:      return "number";
        case TOKEN_DECL:        return "declaration";
        case TOKEN_STRING:      return "string";
        case TOKEN_EOF:         return "<EOF>";
        default:                return "[undefined]";
    }
}

void print_line_with_underline(const char *filename, Span span) {
    char *file_content = read_whole_file(filename);
    long new_lines = 0;
    long lines = (long)span.line;
    while (new_lines < max(lines - 3, 0) && *file_content != '\0') {
        if (*file_content == 10) {
            new_lines++;
        }
        file_content++;
    }
    char *line = strsep(&file_content, "\n");
    char offset[32];
    for (long i = lines - 3; i < lines; i++) {
        if (i < 0) continue;
        sprintf(offset, "%lu | ", i + 1);
        printf("%s%s\n", offset, line);

        line = strsep(&file_content, "\n");
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
        Symbol symbol = img.sym_table[i];
        printf("%s", symbol.name);
        for (size_t j = strlen(symbol.name); j < longest_name_len; j++) {
            printf(".");
        }
        printf(":    0x%04x %s", symbol.address, (symbol.is_resolved) ? "" : "(unresolved)");
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

void print_instr(Instr instr) {
    printf("%s ", instr.name);
    for (size_t i = 0; i < vector_size(instr.ops); i++) {
        if (i != vector_size(instr.ops) - 1) {
            printf(", ");
        }
    }
    printf("\n");
}

void print_decl(Decl decl) {
    printf("%s %s\n", decl.kind.value, decl.value.value);
}

void print_label(Label lbl) {
    if (lbl.name == NULL) {
        printf("No label\n");
        return;
    }
    printf("Label `%s`%s:\n", lbl.name, lbl.is_data ? " (data)" : "");
    if (vector_size(lbl.instructions) == 0) {
        printf("No content\n");
        return;
    }
    if (!lbl.is_data) {
        for (size_t i = 0; i < vector_size(lbl.instructions); i++) {
            printf("    ");
            print_instr(lbl.instructions[i]);
        }
    } else {
        for (size_t i = 0; i < vector_size(lbl.declarations); i++) {
            printf("    ");
            print_decl(lbl.declarations[i]);
        }
    }
    printf("\n");
}

void dump_image(Image image, const char *filename) {
    FILE *fp = fopen(filename, "wb");
    fwrite(image.data, vector_size(image.data), sizeof(byte), fp);
    fclose(fp);
}
