#ifndef __ASM_IMAGE_H
#define __ASM_IMAGE_H

#include "common/arch.h"
#include "common/vector.h"
#include "parser.h"

typedef struct {
    word address;
    Span pos;
} SymbolUsage;

typedef struct {
    char *name;
    word address;
    vector(SymbolUsage) unresolved_usages; // all places where this name is used
    bool is_resolved;
} Symbol;

Symbol new_symbol(const char *name, bool is_resolved);
void symbol_add_usage(Symbol *symbol, word usage, Span pos);
void free_symbol(void *entry);

// Intermediate Representation of the program
typedef struct {
    vector(Symbol) sym_table;
    vector(Label) labels;
    vector(Directive) diresctives;
    vector(byte) buffer;
} Program;

Program new_program(void);

void program_add_label(Program *prog, Label lbl);
void program_add_directive(Program *prog, Directive dir);
void program_codegen(Program *prog);
void program_add_usage(Program *prog, const char *name, Span name_pos, word usage_address);
void program_add_definition(Program *prog, const char *name, word def_address);
void program_resolve_names(Program *prog);
Symbol *program_get_symbol(Program prog, const char *name);
void program_codegen_directive(Program *prog, Directive dir);
void program_codegen_data(Program *prog, Decl decl);
void program_codegen_code(Program *prog, Instr instr);
size_t program_size(Program prog);
void program_check_unresolved_names(Program prog);

void free_program(void *prog);

#endif
