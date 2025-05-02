#ifndef __ASM_IMAGE_H
#define __ASM_IMAGE_H

#include "assembler/parser.h"
#include "common/arch.h"
#include "common/vector.h"

typedef struct {
    char *name;
    word address;
    vector(word) unresolved_usages; // all places where this name is used
    bool is_resolved;
} Symbol;

Symbol new_symbol(const char *name, bool is_resolved);
void symbol_add_usage(Symbol *symbol, word usage);
void free_symbol(void *entry);

typedef struct {
    vector(Symbol) sym_table;
    vector(Label) labels;
    vector(byte) buffer;
} Image;

Image new_image(void);

void image_add_label(Image *img, Label lbl);
void image_codegen(Image *img);
void image_add_usage(Image *image, const char *name, word usage_address);
void image_add_definition(Image *image, const char *name, word def_address);
void image_resolve_names(Image *image);
Symbol *image_get_symbol(Image image, const char *name);
void image_codegen_data(Image *image, Decl decl);
void image_codegen_code(Image *image, Instr instr);
size_t image_content_size(Image image);
void image_check_unresolved_names(Image image);

void free_image(void *image);

#endif
