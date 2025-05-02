#include "image.h"
#include "assembler/lexer.h"
#include "common/vector.h"
#include "common/utils.h"
#include "io.h"
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ------------------------------------------------------------------------------------------------

Symbol new_symbol(const char *name, bool is_resolved) {
    assert(name != NULL);
    char *alloced_name = malloc(strlen(name) + 1);
    strcpy(alloced_name, name);
    vector(word) usgaes = NULL;
    return (Symbol) {
        .name = alloced_name,
        .address = 0,
        .unresolved_usages = usgaes,
        .is_resolved = is_resolved,
    };
}

void symbol_add_usage(Symbol *symbol, word usage) {
    vector_push_back(symbol->unresolved_usages, usage);
}

void free_symbol(void *entry) {
    Symbol *s = (Symbol *)entry;
    if (s->name) {
        free(((Symbol *)entry)->name);
    }
    free_vector(&s->unresolved_usages);
}

// ------------------------------------------------------------------------------------------------

Image new_image(void) {
    vector(Symbol) sym_table = NULL;
    vector_set_destructor(sym_table, free_symbol);
    vector(Label) labels = NULL;
    vector_set_destructor(labels, free_label);

    return (Image) {
        .sym_table = sym_table,
        .labels = labels,
        .buffer = NULL,
    };
}

void image_add_label(Image *img, Label lbl) {
    vector_push_back(img->labels, lbl);
    Symbol *symb = image_get_symbol(*img, lbl.name);
    if (!symb) {
        vector_push_back(img->sym_table, new_symbol(lbl.name, false));
    }
}

void image_codegen(Image *image) {
    // Data labels
    foreach(Label, lbl, image->labels) {
        if (!lbl->is_data) continue;
        image_add_definition(image, lbl->name, image_content_size(*image));
        foreach(Decl, decl, lbl->declarations) {
            image_codegen_data(image, *decl);
        }
    }
    // Code labels
    foreach(Label, lbl, image->labels) {
        if (lbl->is_data) continue;
        image_add_definition(image, lbl->name, image_content_size(*image));
        foreach(Instr, instr, lbl->instructions) {
            image_codegen_code(image, *instr);
        }
    }
    image_resolve_names(image);
}

static void image_subst_address(Image *image, word where, word what) {
    image->buffer[where]     = what >> 8;
    image->buffer[where + 1] = what & 0xFF;
}

void image_add_usage(Image *image, const char *name, word usage_address) {
    Symbol *definition = image_get_symbol(*image, name);
    if (!definition) {
        Symbol new = new_symbol(name, false);
        symbol_add_usage(&new, usage_address);
        vector_push_back(image->sym_table, new);
    } else {
        symbol_add_usage(definition, usage_address);
    }
}

void image_add_definition(Image *image, const char *name, word def_address) {
    Symbol *definition = image_get_symbol(*image, name);
    if (!definition) {
        Symbol new = new_symbol(name, true);
        new.address = def_address;
        vector_push_back(image->sym_table, new);
    } else {
        definition->is_resolved = true;
        definition->address = def_address;
    }
}

void image_resolve_names(Image *image) {
    foreach(Symbol, symb, image->sym_table) {
        if (symb->is_resolved) {
            foreach(word, addr, symb->unresolved_usages) {
                image_subst_address(image, *addr, symb->address);
            }
            vector_clean(symb->unresolved_usages);
        }
    }
}

Symbol *image_get_symbol(Image image, const char *name) {
    for (size_t i = 0; i < vector_size(image.sym_table); i++) {
        Symbol s = image.sym_table[i];
        if (strcmp(s.name, name) == 0) {
            return image.sym_table + i;
        }
    }
    return NULL;
}

void image_codegen_data(Image *image, Decl decl) {
    char *UNUSED;
    long value = strtol(decl.value.value, &UNUSED, 10);
    if (strcmp(decl.kind.value, ".align") == 0) {
        for (long i = 0; i < value; i++) {
            vector_push_back(image->buffer, 0);
        }
    }
    if (strcmp(decl.kind.value, ".byte") == 0) {
        vector_push_back(image->buffer, (byte)value);
    }
    if (strcmp(decl.kind.value, ".word") == 0) {
        vector_push_back(image->buffer, value >> 8);
        vector_push_back(image->buffer, (byte)value);
    }
    if (strcmp(decl.kind.value, ".ascii") == 0) {
        for (size_t i = 1; i < strlen(decl.value.value) - 1; i++) {
            vector_push_back(image->buffer, decl.value.value[i]);
        }
    }
}

static void append_number(unsigned long *buffer, size_t *buffer_size, Token number) {
    char *strend;
    long num = strtol(number.value, &strend, 10);
    *buffer <<= 8;
    *buffer |= (word)(num >> 8);
    *buffer <<= 8;
    *buffer |= (word)(num & 0xff);
    *buffer_size += 16;
}

static void append_bit(unsigned long *buffer, size_t *buffer_size, int bit) {
    *buffer <<= 1;
    *buffer |= bit;
    *buffer_size += 1;
}

static void append_register(unsigned long *buffer, size_t *buffer_size, Token reg) {
    *buffer <<= REGISTER_BIT_SIZE;
    *buffer |= get_register_code(reg.value);
    *buffer_size += REGISTER_BIT_SIZE;
}

static void append_ident(unsigned long *buffer, size_t *buffer_size, Image *image, Token ident) {
    image_add_usage(image, ident.value, image_content_size(*image) + *buffer_size / 8);
    *buffer <<= 16;
    *buffer_size += 16;
}

static void append_alignment(unsigned long *buffer, size_t *buffer_size, size_t alignment) {
    *buffer <<= alignment;
    *buffer_size += alignment;
}

static void append_cmp(unsigned long *buffer, size_t *buffer_size, Token cmp) {
    Cmp c = cmp_from_string(cmp.value);
    *buffer <<= 3;
    *buffer |= c;
    *buffer_size += 3;
}

void image_codegen_code(Image *image, Instr instr) {
    size_t instr_bit_size = 0;
    unsigned long instr_bin_repr = get_instr_opcode(instr.name);
    instr_bit_size += OPCODE_BIT_SIZE;

    if (string_in_args(instr.name, 4, "mov", "load", "store", "cmp")) {
        append_register(&instr_bin_repr, &instr_bit_size, instr.ops[0]);
        if (instr.ops[1].type == TOKEN_REG) {
            append_bit(&instr_bin_repr, &instr_bit_size, 0);
            append_register(&instr_bin_repr, &instr_bit_size, instr.ops[1]);
        } else {
            append_bit(&instr_bin_repr, &instr_bit_size, 1);
            append_alignment(&instr_bin_repr, &instr_bit_size, 6);
            if (instr.ops[1].type == TOKEN_NUMBER) {
                append_number(&instr_bin_repr, &instr_bit_size, instr.ops[1]);
            } else {
                append_ident(&instr_bin_repr, &instr_bit_size, image, instr.ops[1]);
            }
        }
    }
    if (string_in_args(instr.name, 9, "add", "sub", "mul", "div", "and", "or", "xor", "shl", "shr"))
    {
        append_register(&instr_bin_repr, &instr_bit_size, instr.ops[0]);
        if (instr.ops[1].type == TOKEN_REG) {
            append_bit(&instr_bin_repr, &instr_bit_size, 0);
            append_register(&instr_bin_repr, &instr_bit_size, instr.ops[1]);
        } else {
            append_bit(&instr_bin_repr, &instr_bit_size, 1);
            append_alignment(&instr_bin_repr, &instr_bit_size, 6);
            if (instr.ops[1].type == TOKEN_NUMBER) {
                append_number(&instr_bin_repr, &instr_bit_size, instr.ops[1]);
            } else {
                append_ident(&instr_bin_repr, &instr_bit_size, image, instr.ops[1]);
            }
        }
    }
    if (string_in_args(instr.name, 3, "not", "push", "pop")) {
        append_register(&instr_bin_repr, &instr_bit_size, instr.ops[0]);
    }
    if (string_in_args(instr.name, 2, "call", "jmp")) {
        append_alignment(&instr_bin_repr, &instr_bit_size, 3);
        append_ident(&instr_bin_repr, &instr_bit_size, image, instr.ops[0]);
    }
    if (strcmp(instr.name, "jif") == 0) {
        append_cmp(&instr_bin_repr, &instr_bit_size, instr.ops[0]);
        append_ident(&instr_bin_repr, &instr_bit_size, image, instr.ops[1]);
    }

    size_t instr_byte_size = instr_bit_size / 8;
    if (instr_bit_size / 8.0 > (int)(instr_bit_size / 8.0)) {
        instr_byte_size += 1;
    }
    instr_bin_repr <<= instr_byte_size * 8 - instr_bit_size;
    for (int i = instr_byte_size - 1; i >= 0; i--) {
        byte mask = 0xff;
        byte b = (instr_bin_repr & (mask << i * 8)) >> i * 8;
        vector_push_back(image->buffer, b);
    }
}

size_t image_content_size(Image image) {
    return vector_size(image.buffer);
}

void image_check_unresolved_names(Image image) {
    foreach(Symbol, sym, image.sym_table) {
        if (!sym->is_resolved) {
            error_unresolved_name(*sym);
        }
    }
}

void free_image(void *image) {
    Image img = *(Image *)image;
    free_vector(&img.sym_table);
    free_vector(&img.labels);
    free_vector(&img.buffer);
}
