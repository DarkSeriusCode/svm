#ifndef __VM_EXEC_H
#define __VM_EXEC_H

#include "common/arch.h"
#include "common/vector.h"
#include "vm/device.h"
#include <stdio.h>

#define STACK_SIZE 64 * 2 // Size in bytes
#define STACK_OFFSET 8

#define MEMORY_SIZE 256 * 256

word read_word_as_big_endian(byte *memory);

// ------------------------------------------------------------------------------------------------

typedef struct {
    word general_registers[13];
    word ip;
    word sp;
    word cf;
    byte *memory;
    size_t program_size;
    size_t stack_size;
    word stack_begging;
    vector(Port) ports;
} VM;

VM new_vm(const char *program_file);
void free_vm(void *vm);

Device *vm_get_device_by_port_id(VM vm, word port_id);
void vm_load_device(VM *vm, const char *device_file);

// Returns 0 if the last instruction was executed, otherwise returns 1
int exec_instr(VM *vm);

void push_in_stack(VM *vm, word value);
word pop_from_stack(VM *vm);

#endif
