#ifndef __VM_EXEC_H
#define __VM_EXEC_H

#include "common/arch.h"
#include <stdio.h>

#define STACK_SIZE 64*2 // Size in bytes
#define STACK_OFFSET 8

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
} VM;

VM new_vm(byte *memory, size_t program_size);
// Returns 0 if the last instruction was executed, otherwise returns 1
int exec_instr(VM *vm);
void push_in_stack(VM *vm, word value);
word pop_from_stack(VM *vm);
// Puts a call frame on top of the stack
void call_function(VM *vm, word func_addr, word ret_addr);
// Takes a call frame from stack and exits from a function
void return_from_function(VM *vm);

// ------------------------------------------------------------------------------------------------

void dump_vm(VM vm, size_t memory_size_to_dump, const char *filename);

#endif
