#include "image.h"
#include "assembler/lexer.h"
#include "common/vector.h"
#include "error.h"
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

// ------------------------------------------------------------------------------------------------

Symbol new_symbol(const char *name, word address, word usage_address, bool is_resolved) {
    assert(name != NULL);
    char *alloced_name = malloc(strlen(name) + 1);
    strcpy(alloced_name, name);
    return (Symbol) {
        .name = alloced_name,
        .address = address,
        .usage_address = usage_address,
        .is_resolved = is_resolved,
    };
}

void free_symbol(void *entry) {
    if (((Symbol *)entry)->name) {
        free(((Symbol *)entry)->name);
    }
}

// ------------------------------------------------------------------------------------------------

Image new_image(void) {
    vector(Symbol) sym_table = NULL;
    vector_set_destructor(sym_table, free_symbol);

    return (Image) {
        .sym_table = sym_table,
        .data = NULL,
    };
}

static void image_subst_address(Image *image, word where, word what) {
    image->data[where]     = what >> 8;
    image->data[where + 1] = what & 0xFF;
}

void image_add_declaration(Image *image, const char *name, word decl_address) {
    Symbol *definition = image_get_symbol(*image, name);
    if (!definition) {
        vector_push_back(image->sym_table, new_symbol(name, 0, decl_address, false));
    } else {
        image_subst_address(image, decl_address, definition->address);
    }
}

void image_add_definition(Image *image, const char *name, word def_address) {
    Symbol *definition = image_get_symbol(*image, name);
    if (definition && !definition->is_resolved) {
        image_subst_address(image, definition->usage_address, def_address);
        definition->address = def_address;
        definition->is_resolved = true;
    }
    if (!definition) {
        vector_push_back(image->sym_table, new_symbol(name, def_address, 0, true));
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

void image_add_data(Image *image, Decl decl) {
    char *UNUSED;
    long value = strtol(decl.value.value, &UNUSED, 10);
    if (strcmp(decl.kind.value, ".align") == 0) {
        if (value < 0) {
            error_negative_alignment_size(decl.span);
        }
        for (long i = 0; i < value; i++) {
            vector_push_back(image->data, 0);
        }
    }
    if (strcmp(decl.kind.value, ".byte") == 0) {
        check_number_bounds(decl.value, 1);
        vector_push_back(image->data, (byte)value);
    }
    if (strcmp(decl.kind.value, ".word") == 0) {
        check_number_bounds(decl.value, 2);
        vector_push_back(image->data, value >> 8);
        vector_push_back(image->data, (byte)value);
    }
    if (strcmp(decl.kind.value, ".ascii") == 0) {
        for (size_t i = 1; i < strlen(decl.value.value) - 1; i++) {
            vector_push_back(image->data, decl.value.value[i]);
        }
    }
}

static void append_reg_to_bin_repr(unsigned long *bin_repr, size_t *offset, Token reg) {
    *bin_repr |= (get_register_code(reg.value) << (*offset -= REGISTER_BIT_SIZE));
}

static void append_bit_to_bin_repr(unsigned long *bin_repr, size_t *offset, int bit) {
    *bin_repr |= (bit << (*offset -= 1));
}

static void append_number_to_bin_repr(unsigned long *bin_repr, size_t *offset, Token number) {
    char *strend;
    word num = (word)strtol(number.value, &strend, 10);
    *bin_repr |= num;
    *offset = 0;
}

static void append_name_placeholder_to_bin_repr(size_t *offset) {
    *offset -= sizeof(word);
}

void image_add_code(Image *image, Instr instr) {
    unsigned long instr_bin_repr = 0;
    size_t instr_size = get_instr_size(instr.name);
    instr_bin_repr |= get_instr_opcode(instr.name);
    size_t offset = instr_size * 8 - OPCODE_BIT_SIZE;
    instr_bin_repr <<= offset;

    for (int i = instr_size - 1; i >= 0; i--) {
        vector_push_back(image->data, 0);
    }

    if (strcmp(instr.name, "mov") == 0) {
        append_reg_to_bin_repr(&instr_bin_repr, &offset, instr.ops[0]);
        append_reg_to_bin_repr(&instr_bin_repr, &offset, instr.ops[1]);
    }
    if (strcmp(instr.name, "movi") == 0) {
        append_reg_to_bin_repr(&instr_bin_repr, &offset, instr.ops[0]);
        size_t address = image_content_size(*image) - 2;
        if (instr.ops[1].type == TOKEN_IDENT) {
            append_bit_to_bin_repr(&instr_bin_repr, &offset, 0);
            image_add_declaration(image, instr.ops[1].value, address);
            append_name_placeholder_to_bin_repr(&offset);
        } else {
            append_bit_to_bin_repr(&instr_bin_repr, &offset, 1);
            append_number_to_bin_repr(&instr_bin_repr, &offset, instr.ops[1]);
        }
    }
    if (strcmp(instr.name, "load") * strcmp(instr.name, "store") == 0) {
        append_reg_to_bin_repr(&instr_bin_repr, &offset, instr.ops[0]);
        // image_content_size returns size without that instruction.
        size_t address = image_content_size(*image) - 2;
        if (instr.ops[1].type == TOKEN_IDENT) {
            append_bit_to_bin_repr(&instr_bin_repr, &offset, 0);
            image_add_declaration(image, instr.ops[1].value, address);
            append_name_placeholder_to_bin_repr(&offset);
        } else if (instr.ops[1].type == TOKEN_NUMBER) {
            check_number_bounds(instr.ops[1], 2);
            append_number_to_bin_repr(&instr_bin_repr, &offset, instr.ops[1]);
        } else {
            append_bit_to_bin_repr(&instr_bin_repr, &offset, 1);
            append_reg_to_bin_repr(&instr_bin_repr, &offset, instr.ops[1]);
        }
    }
    if (strcmp(instr.name, "add") * strcmp(instr.name, "sub") * strcmp(instr.name, "mul")
        * strcmp(instr.name, "div") == 0)
    {
        append_reg_to_bin_repr(&instr_bin_repr, &offset, instr.ops[0]);
        append_reg_to_bin_repr(&instr_bin_repr, &offset, instr.ops[1]);
    }
    if (strcmp(instr.name, "push") * strcmp(instr.name, "pop") == 0) {
        append_reg_to_bin_repr(&instr_bin_repr, &offset, instr.ops[0]);
    }

    size_t data_size = image_content_size(*image);
    for (int i = instr_size - 1; i >= 0; i--) {
        byte b = (byte)(instr_bin_repr >> i*8);
        image->data[data_size - i - 1] |= b;
    }
}

size_t image_content_size(Image image) {
    return vector_size(image.data);
}

void image_check_unresolved_names(Image image) {
    for (size_t i = 0; i < vector_size(image.sym_table); i++) {
        Symbol sym = image.sym_table[i];
        if (!sym.is_resolved) {
            error_unresolved_name(sym);
        }
    }
}

void free_image(void *image) {
    Image img = *(Image *)image;
    free_vector(img.sym_table);
    free_vector(img.data);
}
