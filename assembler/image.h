#ifndef __ASM_IMAGE_H
#define __ASM_IMAGE_H

#include "assembler/parser.h"
#include "common/arch.h"
#include "common/vector.h"

typedef struct {
    char *name;
    word address;
    word usage_address;
    bool is_resolved;
} Symbol;

Symbol new_symbol(const char *name, word address, word usage_address, bool is_resolved);
void free_symbol(void *entry);

typedef struct {
    vector(Symbol) sym_table;
    vector(byte) data;
} Image;

Image new_image(void);

void image_codegen(Image *img, vector(Label) labels);
void image_add_declaration(Image *image, const char *name, word decl_address);
void image_add_definition(Image *image, const char *name, word def_address);
Symbol *image_get_symbol(Image image, const char *name);
void image_codegen_data(Image *image, Decl decl);
void image_codegen_code(Image *image, Instr instr);
size_t image_content_size(Image image);
void image_check_unresolved_names(Image image);

void free_image(void *image);

#endif
