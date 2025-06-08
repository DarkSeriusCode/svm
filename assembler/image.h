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

typedef struct Image {
    vector(Symbol) sym_table;
    vector(Label) labels;
    vector(Directive) diresctives;
    vector(byte) buffer;
} Image;

Image new_image(void);

void image_add_label(Image *img, Label lbl);
void image_add_directive(Image *img, Directive dir);
void image_codegen(Image *img);
void image_add_usage(Image *image, const char *name, Span name_pos, word usage_address);
void image_add_definition(Image *image, const char *name, word def_address);
void image_resolve_names(Image *image);
Symbol *image_get_symbol(Image image, const char *name);
void image_codegen_directive(Image *image, Directive dir);
void image_codegen_data(Image *image, Decl decl);
void image_codegen_code(Image *image, Instr instr);
size_t image_content_size(Image image);
void image_check_unresolved_names(Image image);

void free_image(void *image);

#endif
