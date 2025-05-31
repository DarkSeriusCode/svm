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

#define REG_SP 13
#define REG_IP 14
#define REG_CF 15

typedef struct {
    word registers[16];
    byte *memory;
    size_t program_size;
    size_t stack_size;
    word stack_begging;
    vector(Port) ports;
} VM;

VM new_vm(const char *program_file);
void free_vm(void *vm);

void vm_load_device(VM *vm, const char *device_file, int port_id);
Port *vm_get_port(VM vm, byte port_id);
byte vm_get_free_port_id(VM vm);

// Returns 0 if the last instruction was executed, otherwise returns 1
int exec_instr(VM *vm);

void push_in_stack(VM *vm, word value);
word pop_from_stack(VM *vm);

#endif
