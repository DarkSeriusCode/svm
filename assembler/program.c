#include "program.h"
#include "lexer.h"
#include "common/vector.h"
#include "common/utils.h"
#include "io.h"
#include <assert.h>
#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ------------------------------------------------------------------------------------------------

Symbol new_symbol(const char *name, bool is_resolved) {
    vector(SymbolUsage) usgaes = NULL;

    return (Symbol) {
        .name = strdup(name),
        .address = 0,
        .unresolved_usages = usgaes,
        .is_resolved = is_resolved,
    };
}

void symbol_add_usage(Symbol *symbol, word usage, Span pos) {
    SymbolUsage su = { usage, pos };
    vector_push_back(symbol->unresolved_usages, su);
}

void free_symbol(void *entry) {
    Symbol *s = (Symbol *)entry;
    if (s->name) {
        free(((Symbol *)entry)->name);
    }
    free_vector(&s->unresolved_usages);
}

// ------------------------------------------------------------------------------------------------

Program new_program(void) {
    vector(Symbol) sym_table = NULL;
    vector_set_destructor(sym_table, free_symbol);
    vector(Label) labels = NULL;
    vector_set_destructor(labels, free_label);
    vector(Directive) dirs = NULL;
    vector_set_destructor(dirs, free_directive);

    return (Program) {
        .sym_table = sym_table,
        .labels = labels,
        .diresctives = dirs,
    };
}

void program_add_label(Program *prog, Label lbl) {
    vector_push_back(prog->labels, lbl);
    Symbol *symb = program_get_symbol(*prog, lbl.name);
    if (!symb) {
        vector_push_back(prog->sym_table, new_symbol(lbl.name, false));
    } else {
        error_redefinition(lbl.name, lbl.span);
    }
}

void program_add_directive(Program *prog, Directive dir) {
    vector_push_back(prog->diresctives, dir);
}

void program_add_usage(Program *prog, const char *name, Span name_pos, word usage_address) {
    Symbol *definition = program_get_symbol(*prog, name);
    if (!definition) {
        Symbol new = new_symbol(name, false);
        symbol_add_usage(&new, usage_address, name_pos);
        vector_push_back(prog->sym_table, new);
    } else {
        symbol_add_usage(definition, usage_address, name_pos);
    }
}

void program_add_definition(Program *prog, const char *name, word def_address) {
    Symbol *definition = program_get_symbol(*prog, name);
    if (!definition) {
        Symbol new = new_symbol(name, true);
        new.address = def_address;
        vector_push_back(prog->sym_table, new);
    } else {
        definition->is_resolved = true;
        definition->address = def_address;
    }
}

static void subst_address(vector(byte) buffer, word where, word what) {
    buffer[where]     = what >> 8;
    buffer[where + 1] = what & 0xFF;
}

ExecFile program_compile(Program *prog) {
    ExecFile ef = new_execfile();
    // Compile directives
    vector(byte) compiled_dirs = NULL;
    foreach(Directive, dir, prog->diresctives) {
        program_compile_directive(&compiled_dirs, *dir);
    }
    if (!vector_empty(compiled_dirs)) {
        execfile_add_section(&ef, "directives", compiled_dirs);
        free_vector(&compiled_dirs);
    }

    // Compile program
    vector(byte) compiled_program = NULL;
    foreach(Label, lbl, prog->labels) {
        if (!lbl->is_data) continue;
        program_add_definition(prog, lbl->name, vector_size(compiled_program));
        foreach(Decl, decl, lbl->declarations) {
            program_compile_data(prog, &compiled_program, *decl);
        }
    }
    foreach(Label, lbl, prog->labels) {
        if (lbl->is_data) continue;
        program_add_definition(prog, lbl->name, vector_size(compiled_program));
        foreach(Instr, instr, lbl->instructions) {
            program_compile_code(prog, &compiled_program, *instr);
        }
    }
    Symbol *entry_point = program_get_symbol(*prog, ENTRY_POINT_NAME);
    if (!entry_point || !entry_point->is_resolved) {
        error_no_entry();
    }
    program_resolve_names(prog, &compiled_program);
    if (!vector_empty(compiled_program)) {
        execfile_add_section(&ef, "program", compiled_program);
        free_vector(&compiled_program);
    }

    // Compile symbol table
    vector(byte) compiled_sym_table = NULL;
    program_compile_symbol_table(*prog, &compiled_sym_table);
    if (!vector_empty(compiled_sym_table)) {
        execfile_add_section(&ef, "symbols", compiled_sym_table);
        free_vector(&compiled_sym_table);
    }

    return ef;
}

void program_compile_symbol_table(Program prog, vector(byte) *buffer) {
    foreach(Symbol, symb, prog.sym_table) {
        for (size_t i = 0; i < strlen(symb->name) + 1; i++) {
            vector_push_back(*buffer, symb->name[i]);
        }
        vector_push_word_back(*buffer, symb->address);
    }
}

void program_compile_directive(vector(byte) *buffer, Directive dir) {
    byte dir_code = (byte)dir.opcode;
    vector_push_back(*buffer, dir_code);
    if (dir.opcode == DIR_USE) {
        for (size_t i = 0; i < strlen(dir.params[0].value) + 1; i++) {
            vector_push_back(*buffer, dir.params[0].value[i]);
        }
        char *unused;
        long port = strtol(dir.params[1].value, &unused, 10);
        vector_push_back(*buffer, (byte)port);
    }
}

void program_compile_data(Program *prog, vector(byte) *buffer, Decl decl) {
    char *UNUSED;
    long value = strtol(decl.value.value, &UNUSED, 10);
    if (strcmp(decl.kind.value, ".align") == 0) {
        for (long i = 0; i < value; i++) {
            vector_push_back(*buffer, 0);
        }
    }
    if (strcmp(decl.kind.value, ".byte") == 0) {
        vector_push_back(*buffer, (byte)value);
    }
    if (strcmp(decl.kind.value, ".word") == 0) {
        vector_push_word_back(*buffer, value);
    }
    if (strcmp(decl.kind.value, ".ascii") == 0) {
        for (size_t i = 0; i < strlen(decl.value.value); i++) {
            vector_push_back(*buffer, decl.value.value[i]);
        }
    }
    if (strcmp(decl.kind.value, ".sizeof") == 0) {
        Symbol *sym = program_get_symbol(*prog, decl.value.value);
        if (sym == NULL) {
            program_add_usage(prog, decl.value.value, decl.value.span, vector_size(*buffer));
            return;
        }
        foreach(Label, lbl, prog->labels) {
            if (lbl->is_data && strcmp(lbl->name, decl.value.value) == 0) {
                vector_push_word_back(*buffer, lbl->data_size);
            }
        }
    }
}

static void append_byte(unsigned long *buffer, size_t *buffer_size, Token number) {
    char *strend;
    long num = strtol(number.value, &strend, 10);
    *buffer <<= 8;
    *buffer |= num & 0xFF;
    *buffer_size += 8;
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

static void append_ident(unsigned long *buffer, size_t *buffer_size, Program *prog,
                         vector(byte) prog_buffer, Token ident)
{
    program_add_usage(prog, ident.value, ident.span, vector_size(prog_buffer) + *buffer_size / 8);
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

void program_compile_code(Program *prog, vector(byte) *buffer, Instr instr) {
    size_t instr_bit_size = 0;
    uint64_t instr_bin_repr = (uint64_t)instr.opcode;
    instr_bit_size += OPCODE_BIT_SIZE;

    if (instropcode_in_args(instr.opcode, 4, INSTR_MOV, INSTR_LD, INSTR_ST, INSTR_CMP)) {
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
                append_ident(&instr_bin_repr, &instr_bit_size, prog, *buffer, instr.ops[1]);
            }
        }
    }
    if (instropcode_in_args(instr.opcode, 9, INSTR_ADD, INSTR_SUB, INSTR_MUL, INSTR_DIV, INSTR_AND,
                                             INSTR_OR, INSTR_OR, INSTR_SHL, INSTR_SHR))
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
                append_ident(&instr_bin_repr, &instr_bit_size, prog, *buffer, instr.ops[1]);
            }
        }
    }
    if (instropcode_in_args(instr.opcode, 3, INSTR_NOT, INSTR_PUSH, INSTR_POP)) {
        append_register(&instr_bin_repr, &instr_bit_size, instr.ops[0]);
    }
    if (instropcode_in_args(instr.opcode, 2, INSTR_CALL, INSTR_JMP)) {
        append_alignment(&instr_bin_repr, &instr_bit_size, 3);
        append_ident(&instr_bin_repr, &instr_bit_size, prog, *buffer, instr.ops[0]);
    }
    if (instr.opcode == INSTR_JIF) {
        append_cmp(&instr_bin_repr, &instr_bit_size, instr.ops[0]);
        append_ident(&instr_bin_repr, &instr_bit_size, prog, *buffer, instr.ops[1]);
    }
    if (instropcode_in_args(instr.opcode, 2, INSTR_OUT, INSTR_IN)) {
        append_byte(&instr_bin_repr, &instr_bit_size, instr.ops[0]);

        if (instr.ops[1].type == TOKEN_REG) append_bit(&instr_bin_repr, &instr_bit_size, 0);
        else append_bit(&instr_bin_repr, &instr_bit_size, 1);
        if (instr.ops[2].type == TOKEN_REG) append_bit(&instr_bin_repr, &instr_bit_size, 0);
        else append_bit(&instr_bin_repr, &instr_bit_size, 1);

        for (size_t i = 1; i <= 2; i++) {
            Token op = instr.ops[i];
            if (op.type == TOKEN_IDENT) {
                append_alignment(&instr_bin_repr, &instr_bit_size, 1);
                append_ident(&instr_bin_repr, &instr_bit_size, prog, *buffer, op);
            } else if (op.type == TOKEN_NUMBER) {
                append_alignment(&instr_bin_repr, &instr_bit_size, 1);
                append_number(&instr_bin_repr, &instr_bit_size, op);
            } else {
                append_register(&instr_bin_repr, &instr_bit_size, op);
            }
        }
    }

    size_t instr_byte_size = instr_bit_size / 8;
    if (instr_bit_size / 8.0 > (int)(instr_bit_size / 8.0)) {
        instr_byte_size += 1;
    }
    instr_bin_repr <<= instr_byte_size * 8 - instr_bit_size;
    for (int i = instr_byte_size - 1; i >= 0; i--) {
        unsigned long mask = 0xff;
        byte b = (instr_bin_repr & (mask << i * 8)) >> i * 8;
        vector_push_back(*buffer, b);
    }
}

void program_resolve_names(Program *prog, vector(byte) *buffer) {
    foreach(Symbol, symb, prog->sym_table) {
        if (symb->is_resolved) {
            foreach(SymbolUsage, usgae, symb->unresolved_usages) {
                subst_address(*buffer, usgae->address, symb->address);
            }
            vector_clean(symb->unresolved_usages);
        }
    }
}

Symbol *program_get_symbol(Program prog, const char *name) {
    foreach (Symbol, s, prog.sym_table) {
        if (strcmp(s->name, name) == 0) {
            return s;
        }
    }
    return NULL;
}

void program_check_unresolved_names(Program prog) {
    foreach(Symbol, sym, prog.sym_table) {
        if (!sym->is_resolved) {
            error_unresolved_name(*sym);
        }
    }
}

void free_program(void *prog) {
    Program p = *(Program *)prog;
    free_vector(&p.sym_table);
    free_vector(&p.labels);
    free_vector(&p.diresctives);
}
