#ifndef __ASM_IMAGE_H
#define __ASM_IMAGE_H

#include "common/arch.h"
#include "common/vector.h"
#include "lexer.h"

typedef struct {
    const char *name;
    Token value;
    byte size;
    Span pos;
} Decl;

Decl new_decl(const char *name, Token value, byte size, Span pos);

typedef struct {
    const char *name;
    vector(Token) ops;
    Span pos;
} Instr;

Instr new_instr(const char *name, vector(Token) ops, Span pos);

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
    vector(Instr) instructions;
    vector(Decl) declarations;
    vector(byte) data;
} Image;

Image new_image(void);

void image_add_decl(Image *image, const char *name, Token value, byte size, Span pos);
void image_add_instr(Image *image, const char *name, vector(Token) ops, Span pos);
Decl *image_get_decl(Image image, const char *name);
Instr *image_get_instr(Image image, const char *name);

void image_add_declaration(Image *image, const char *name, word decl_address);
void image_add_definition(Image *image, const char *name, word def_address);
Symbol *image_get_symbol(Image image, const char *name);
void image_add_data(Image *image, vector(byte) new_data);

void image_analize(Image img);
void check_number_bounds(Token num, size_t should_has_size);
void check_single_op(Token op, size_t expected_types_count, ...);
void check_ident_size(Image img, Token ident, size_t expected_size);
void check_instr_ops(Image img, Instr instr);

void free_image(void *image);

#endif
