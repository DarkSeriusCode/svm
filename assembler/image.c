#include "image.h"
#include "common/vector.h"
#include "error.h"
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

Decl new_decl(const char *name, Token value, byte size, Span pos) {
    assert(size == 1 || size == 2);
    char *allocated_str = malloc(strlen(name) + 1);
    strcpy(allocated_str, name);
    return (Decl){ allocated_str, value, size, pos };
}

Instr new_instr(const char *name, vector(Token) ops, Span pos) {
    char *allocated_str = malloc(strlen(name) + 1);
    strcpy(allocated_str, name);
    vector(Token) new_ops = NULL;
    for (size_t i = 0; i < vector_size(ops); i++) {
        vector_push_back(new_ops, copy_token(ops[i]));
    }
    return (Instr){ allocated_str, new_ops, pos };
}

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
    vector(Decl) decls = NULL;
    vector(Instr) instrs = NULL;

    return (Image) {
        .sym_table = sym_table,
        .declarations = decls,
        .instructions = instrs,
        .data = NULL,
    };
}

void image_add_decl(Image *image, const char *name, Token value, byte size, Span pos) {
    if (!image_get_decl(*image, name)) {
        vector_push_back(image->declarations, new_decl(name, value, size, pos));
    } else {
        error_redefinition(name, pos);
    }
}

void image_add_instr(Image *image, const char *name, vector(Token) ops, Span pos) {
    vector_push_back(image->instructions, new_instr(name, ops, pos));
}

Decl *image_get_decl(Image image, const char *name) {
    Decl *found_res;
    vector_find_str_by(image.declarations, .name, name, found_res);
    return found_res;
}

Instr *image_get_instr(Image image, const char *name) {
    Instr *found_res;
    vector_find_str_by(image.instructions, .name, name, found_res);
    return found_res;
}

void image_add_declaration(Image *image, const char *name, word decl_address) {
    if (!image_get_symbol(*image, name))
        vector_push_back(image->sym_table, new_symbol(name, 0, decl_address, false));
}

void image_add_definition(Image *image, const char *name, word def_address) {
    Symbol *definition = image_get_symbol(*image, name);
    if (definition && !definition->is_resolved) {
        image->data[definition->usage_address]     = def_address >> 8;
        image->data[definition->usage_address + 1] = def_address & 0xFF;
        definition->address = def_address;
        definition->is_resolved = true;
    }
    if (!definition) {
        vector_push_back(image->sym_table, new_symbol(name, def_address, 0, true));
    }
}

Symbol *image_get_symbol(Image image, const char *name) {
    Symbol *found_res;
    vector_find_str_by(image.sym_table, .name, name, found_res);
    return found_res;
}

void image_add_data(Image *image, vector(byte) new_data) {
    for (size_t i = 0; i < vector_size(new_data); i++)
        vector_push_back(image->data, new_data[i]);
}

void image_analize(Image img) {
    Decl decl;
    for (size_t i = 0; i < vector_size(img.declarations); i++) {
        decl = img.declarations[i];
        if (decl.value.type == TOKEN_NUMBER) {
            check_number_bounds(decl.value, decl.size);
        }
    }
    Instr instr;
    for (size_t i = 0; i < vector_size(img.instructions); i++) {
        instr = img.instructions[i];
        check_instr_ops(img, instr);
    }
}

void check_number_bounds(Token num, size_t should_has_size) {
    assert(should_has_size == 1 || should_has_size == 2);
    char *strend;
    long n = strtol(num.value, &strend, 10);
    long lower_bound = 0, upper_bound = 0;
    if (should_has_size == 1) {
        if (n < 0) {
            lower_bound = CHAR_MIN;
            upper_bound = CHAR_MAX;
        } else {
            lower_bound = 0;
            upper_bound = UCHAR_MAX;
        }
    } else {
        if (n < 0) {
            lower_bound = SHRT_MIN;
            upper_bound = SHRT_MAX;
        } else {
            lower_bound = 0;
            upper_bound = USHRT_MAX;
        }
    }
    if (!(lower_bound < n && n < upper_bound)) {
        warning_number_out_of_bounds(n, lower_bound, upper_bound, num.span);
    }
}

void check_single_op(Token op, size_t expected_types_count, ...) {
    va_list args;
    va_start(args, expected_types_count);
    vector(TokenType) types = NULL;
    for (size_t i = 0; i < expected_types_count; i++) {
        TokenType tt = va_arg(args, TokenType);
        vector_push_back(types, tt);
        if (op.type == tt) {
            return;
        }
    }
    error_invalid_operand_in_vec(op.type, op.span, types);
    va_end(args);
}

void check_ident_size(Image img, Token ident, size_t expected_size) {
    Decl *decl = image_get_decl(img, ident.value);
    if (!decl) {
        error_undefined_identifier(ident);
    }
    if (decl->size != expected_size) {
        warning_wrong_ident_size(ident, *decl, expected_size);
    }
}

void check_instr_ops(Image img, Instr instr) {
    const char *instr_name = instr.name;
    vector(Token) ops = instr.ops;

    if (strcmp(instr_name, "loadb") * strcmp(instr_name, "storeb") == 0) {
        check_single_op(ops[0], 1, TOKEN_REG);
        check_single_op(ops[1], 1, TOKEN_IDENT);
        check_ident_size(img, ops[1], 1);
    }
    if (strcmp(instr_name, "loadw") * strcmp(instr_name, "storew") == 0) {
        check_single_op(ops[0], 1, TOKEN_REG);
        check_single_op(ops[1], 1, TOKEN_IDENT);
        check_ident_size(img, ops[1], 2);
    }
    if (strcmp(instr_name, "add") * strcmp(instr_name, "sub")
        * strcmp(instr_name, "mul") * strcmp(instr_name, "div") == 0)
    {
        check_single_op(ops[0], 1, TOKEN_REG);
        check_single_op(ops[1], 1, TOKEN_REG);
    }
    if (strcmp(instr_name, "mov") == 0) {
        check_single_op(ops[0], 1, TOKEN_REG);
        check_single_op(ops[1], 2, TOKEN_REG, TOKEN_NUMBER);
        if (ops[1].type == TOKEN_NUMBER) {
            check_number_bounds(ops[1], 2);
        }
    }
}

void free_image(void *image) {
    Image img = *(Image *)image;
    free_vector(img.sym_table);
    free_vector(img.instructions->ops);
    free_vector(img.instructions);
    free_vector(img.declarations);
    free_vector(img.data);
}
