#include "image.h"
#include "common/io.h"
#include "common/vector.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

NameEntry new_name_entry(const char *name, word address, word usage_address, bool is_resolved) {
    assert(name != NULL);
    char *alloced_name = malloc(strlen(name) + 1);
    strcpy(alloced_name, name);
    return (NameEntry) {
        .name = alloced_name,
        .address = address,
        .usage_address = usage_address,
        .is_resolved = is_resolved,
    };
}

void free_name_entry(void *entry) {
    if (((NameEntry *)entry)->name) {
        free(((NameEntry *)entry)->name);
    }
}

// ------------------------------------------------------------------------------------------------

Image new_image(void) {
    vector(NameEntry) sym_table = NULL;
    vector_set_destructor(sym_table, free_name_entry);

    return (Image) {
        .sym_table = sym_table,
        .data = NULL,
    };
}

void image_add_declaration(Image *image, const char *name, word decl_address) {
    if (!image_get_name(*image, name)) {
        vector_push_back(image->sym_table, new_name_entry(name, 0, decl_address, false));
    }
}

void image_add_definition(Image *image, const char *name, word def_address) {
    NameEntry *definition = image_get_name(*image, name);
    if (definition && !definition->is_resolved) {
        image->data[definition->usage_address]     = def_address >> 8;
        image->data[definition->usage_address + 1] = def_address & 0xFF;
        definition->address = def_address;
        definition->is_resolved = true;
    }
    if (!definition) {
        vector_push_back(image->sym_table, new_name_entry(name, def_address, 0, true));
    }
}

NameEntry *image_get_name(Image image, const char *name) {
    for (size_t i = 0; i < vector_size(image.sym_table); i++) {
        if (strcmp(name, image.sym_table[i].name) == 0) {
            return &image.sym_table[i];
        }
    }
    return NULL;
}

void image_add_data(Image *image, vector(byte) new_data) {
    for (size_t i = 0; i < vector_size(new_data); i++) {
        vector_push_back(image->data, new_data[i]);
    }
}

void free_image(void *image) {
    Image img = *(Image *)image;
    free_vector(img.sym_table);
    free_vector(img.data);
}
