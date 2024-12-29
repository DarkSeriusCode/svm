#ifndef __ASM_IMAGE_H
#define __ASM_IMAGE_H

#include "common/arch.h"
#include "common/vector.h"

typedef struct {
    char *name;
    word address;
    word usage_address;
    bool is_resolved;
} NameEntry;

NameEntry new_name_entry(const char *name, word address, word usage_address, bool is_resolved);
void free_name_entry(void *entry);

typedef struct {
    vector(NameEntry) sym_table;
    vector(byte) data;
} Image;

Image new_image(void);
void image_add_declaration(Image *image, const char *name, word decl_address);
void image_add_definition(Image *image, const char *name, word def_address);
NameEntry *image_get_name(Image image, const char *name);
void image_add_data(Image *image, vector(byte) new_data);
void free_image(void *image);

#endif
