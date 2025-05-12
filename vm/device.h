#ifndef __VM_LOADER_H
#define __VM_LOADER_H

#include "common/arch.h"

typedef word(InitFunc)(byte *);
typedef word(FiniFunc)(void);
typedef word(ReadFunc)(word, word);
typedef word(WriteFunc)(word, word);

typedef struct {
    void *dl;
    const char *filename;
    InitFunc *init;
    FiniFunc *fini;
    ReadFunc *read;
    WriteFunc *write;
} Device;

Device new_device(const char *filename);
void free_device(void *dev);

typedef struct {
    word id;
    Device device;
} Port;

#endif
