#include "sex.h"
#include "common/str.h"
#include "io.h"
#include <stdio.h>

SectionHeader new_section(const char *name, uint32_t offset, word size) {
    return (SectionHeader) { new_string(name), offset, size };
}

void free_section(void *sec) {
    SectionHeader s = *(SectionHeader *)sec;
    free_string(&s);
}

ExecFile new_execfile(void) {
    vector(SectionHeader) sections = NULL;
    vector_set_destructor(sections, free_section);
    return (ExecFile) { sections, NULL };
}

void execfile_add_section(ExecFile *ef, const char *name, vector(byte) data) {
    size_t section_offset = vector_size(ef->content);
    size_t section_size = vector_size(data);
    foreach(byte, i, data) {
        vector_push_back(ef->content, *i);
    }
    vector_push_back(ef->sections, new_section(name, section_offset, section_size));
}

void write_execfile(ExecFile ef, const char *filepath) {
    FILE *fp = fopen(filepath, "w");

    // Magic
    byte magic[] = { 'S', 'E', 'X', 0 };
    fwrite(magic, sizeof(magic), sizeof(byte), fp);

    // Amount of sections
    word section_count = (word)vector_size(ef.sections);
    fwrite(&section_count, 1, sizeof(word), fp);

    // Section headers
    foreach(SectionHeader, header, ef.sections) {
        fwrite(header->name, strlen(header->name) + 1, sizeof(char), fp);
        fwrite(&header->offset, 1, sizeof(header->offset), fp);
        fwrite(&header->size, 1, sizeof(header->size), fp);
    }
    fwrite(ef.content, vector_size(ef.content), vector_item_size(ef.content), fp);
    fclose(fp);
}

ExecFile read_execfile(const char *filepath) {
    FILE *fp = fopen(filepath, "r");
    ExecFile ef = new_execfile();

    uint32_t magic;
    fread(&magic, 1, sizeof(magic), fp);
    if (magic != 0x00584553) {
        error_invalid_file_format(filepath);
    }
    word section_count;
    fread(&section_count, 1, sizeof(word), fp);

    vector(SectionHeader) sections = NULL;
    string name = NULL;
    char c;
    for (word i = 0; i < section_count; i++) {
        while ( (c = fgetc(fp)) != '\0' ) string_push_char(&name, c);
        uint32_t offset;
        fread(&offset, 1, sizeof(offset), fp);
        word size;
        fread(&size, 1, sizeof(size), fp);
        vector_push_back(sections, new_section(name, offset, size));
        vector_clean(name);
    }
    free_string(&name);
    vector(byte) content = NULL;
    while (!feof(fp)) vector_push_back(content, fgetc(fp));
    fclose(fp);

    ef.content = content;
    ef.sections = sections;
    return ef;
}

SectionHeader *execfile_get_section(ExecFile ef, const char *section_name) {
    foreach (SectionHeader, s, ef.sections) {
        if (strcmp(s->name, section_name) == 0) {
            return s;
        }
    }
    return NULL;
}

vector(byte) execfile_get_section_content(ExecFile ef, const char *section_name) {
    vector(byte) section_content = NULL;
    SectionHeader *section = execfile_get_section(ef, section_name);
    if (section == NULL) {
        return section_content;
    }
    vector_reserve(section_content, section->size);
    for (size_t i = 0; i < section->size; i++) {
        vector_push_back(section_content, ef.content[section->offset + i]);
    }
    return section_content;
}

void free_execfile(void *exec_file) {
    ExecFile ef = *(ExecFile *)exec_file;
    free_vector(&ef.sections);
    free_vector(&ef.content);
}
