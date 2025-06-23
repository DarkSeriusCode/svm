// SEX stands for Simple EXecutable. It doesn't mean the action you've never done
#ifndef __COMMON_SEX_H
#define __COMMON_SEX_H

#include "common/arch.h"
#include "common/str.h"
#include "common/vector.h"
#include <stdint.h>

typedef struct {
    string name;
    uint32_t offset;
    word size;
} SectionHeader;

SectionHeader new_section(const char *name, uint32_t offset, word size);
void free_section(void *sec);

typedef struct {
    vector(SectionHeader) sections;
    vector(byte) content;
} ExecFile;

ExecFile new_execfile(void);
void execfile_add_section(ExecFile *ef, const char *name, vector(byte) data);
void write_execfile(ExecFile ef, const char *filepath);
ExecFile read_execfile(const char *filepath);
SectionHeader *execfile_get_section(ExecFile ef, const char *section_name);
vector(byte) execfile_get_section_content(ExecFile ef, const char *section_name);
void free_execfile(void *exec_file);

#endif
