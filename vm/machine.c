#include "machine.h"
#include "common/io.h"
#include "common/arch.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

word read_word_as_big_endian(byte *memory) {
    word w = memory[0];
    w <<= 8;
    w |= memory[1];
    return w;
}

unsigned long read_ulong_as_big_endian(byte *memory) {
    unsigned long buffer = memory[0];
    for (size_t i = 1; i < sizeof(unsigned long); i++) {
        buffer <<= 8;
        buffer |= memory[i];
    }
    return buffer;
}

static word read_bits(unsigned long *buffer, size_t *read_bits_count, size_t size) {
    word data = (*buffer >> (sizeof(unsigned long) - sizeof(word)) * 8);
    data >>= sizeof(word) * 8 - size;
    *buffer <<= size;
    *read_bits_count += size;
    return data;
}
#define read_opcode(buffer, read_count)    read_bits(buffer, read_count, OPCODE_BIT_SIZE)
#define read_register(buffer, read_count)  read_bits(buffer, read_count, REGISTER_BIT_SIZE)
#define read_flag(buffer, read_count)      read_bits(buffer, read_count, 1)
#define read_number(buffer, read_count)    read_bits(buffer, read_count, NUMBER_BIT_SIZE)
#define skip_alignment(buffer, read_count, alignment)  read_bits(buffer, read_count, alignment)

// Perform binary operation in instructions like <reg> <reg/imm>
#define reg_reg_binop(buffer, read_bits_count, op) \
    do {\
        word src_reg = read_register(buffer, read_bits_count); \
        word flag = read_flag(buffer, read_bits_count); \
        short value; \
        if (flag) { \
            skip_alignment(buffer, read_bits_count, 6); \
            value = read_number(buffer, read_bits_count); \
        } else { \
            value = vm->general_registers[read_register(buffer, read_bits_count)]; \
        } \
        vm->general_registers[src_reg] op value; \
    } while(0);

// ------------------------------------------------------------------------------------------------

VM new_vm(byte *memory, size_t program_size) {
    VM vm = {
        .program_size = program_size,
        .stack_begging = program_size + STACK_OFFSET + STACK_SIZE,
        .memory = memory,
        .cf = 0,
        .sp = program_size + STACK_OFFSET + STACK_SIZE,
        .ip = read_word_as_big_endian(memory),
    };
    memset(vm.general_registers, 0, sizeof(vm.general_registers));
    push_in_stack(&vm, program_size);

    return vm;
}

int exec_instr(VM *vm) {
    if (vm->ip >= vm->program_size) {
        return 0;
    }
    byte *mem = vm->memory + vm->ip;
    unsigned long buffer = read_ulong_as_big_endian(mem);
    size_t read_bits_count = 0;
    byte opcode = read_bits(&buffer, &read_bits_count, OPCODE_BIT_SIZE);
    switch (opcode) {
        // mov
        case 0b00001: {
            word dest_reg = read_register(&buffer, &read_bits_count);
            word flag = read_flag(&buffer, &read_bits_count);
            word value;
            if (flag) {
                skip_alignment(&buffer, &read_bits_count, 6);
                value = read_number(&buffer, &read_bits_count);
            } else {
                value = vm->general_registers[read_register(&buffer, &read_bits_count)];
            }
            vm->general_registers[dest_reg] = value;
        }; break;

        // load
        case 0b00010: {
            word dest_reg = read_register(&buffer, &read_bits_count);
            word flag = read_flag(&buffer, &read_bits_count);
            word addr;
            if (flag) {
                skip_alignment(&buffer, &read_bits_count, 6);
                addr = read_number(&buffer, &read_bits_count);
            } else {
                addr = vm->general_registers[read_register(&buffer, &read_bits_count)];
            }
            vm->general_registers[dest_reg] = read_word_as_big_endian(vm->memory + addr);
        }; break;

        // store
        case 0b00011: {
            word src_reg = read_register(&buffer, &read_bits_count);
            word flag = read_flag(&buffer, &read_bits_count);
            word addr;
            if (flag) {
                skip_alignment(&buffer, &read_bits_count, 6);
                addr = read_number(&buffer, &read_bits_count);
            } else {
                addr = vm->general_registers[read_register(&buffer, &read_bits_count)];
            }
            word value = vm->general_registers[src_reg];
            vm->memory[addr++] = value >> 8;
            vm->memory[addr] = value & 0xff;
        }; break;

        // add, sub, mul, div, and, or, xor, shl, shr
        case 0b00100: reg_reg_binop(&buffer, &read_bits_count, +=); break;
        case 0b00101: reg_reg_binop(&buffer, &read_bits_count, -=); break;
        case 0b00110: reg_reg_binop(&buffer, &read_bits_count, *=); break;
        case 0b00111: reg_reg_binop(&buffer, &read_bits_count, /=); break;
        case 0b01101: reg_reg_binop(&buffer, &read_bits_count, &=); break;
        case 0b01110: reg_reg_binop(&buffer, &read_bits_count, |=); break;
        case 0b01111: reg_reg_binop(&buffer, &read_bits_count, ^=); break;
        case 0b10000: reg_reg_binop(&buffer, &read_bits_count, <<=); break;
        case 0b10001: reg_reg_binop(&buffer, &read_bits_count, >>=); break;

        // not
        case 0b01000: {
            word reg = read_register(&buffer, &read_bits_count);
            vm->general_registers[reg] = ~vm->general_registers[reg];
        }; break;

        // push
        case 0b01001: {
            word reg = read_register(&buffer, &read_bits_count);
            push_in_stack(vm, vm->general_registers[reg]);
        }; break;

        // pop
        case 0b01010: {
            word reg = read_register(&buffer, &read_bits_count);
            word value = pop_from_stack(vm);
            vm->general_registers[reg] = value;
        }; break;

        // call
        case 0b01011: {
            skip_alignment(&buffer, &read_bits_count, 3);
            word func_addr = read_number(&buffer, &read_bits_count);
            word ret_addr = vm->ip + 3;
            call_function(vm, func_addr, ret_addr);
            return 1;
        }; break;

        // ret
        case 0b01100: {
            return_from_function(vm);
            return 1;
        }; break;

        // jmp
        case 0b10010: {
            skip_alignment(&buffer, &read_bits_count, 3);
            word addr = read_number(&buffer, &read_bits_count);
            vm->ip = addr;
            return 1;
        }; break;

        // cmp
        case 0b10011: {
            word a = vm->general_registers[read_register(&buffer, &read_bits_count)];
            word flag = read_flag(&buffer, &read_bits_count);
            short b;
            if (flag) {
                skip_alignment(&buffer, &read_bits_count, 6);
                b = read_number(&buffer, &read_bits_count);
            } else {
                b = vm->general_registers[read_register(&buffer, &read_bits_count)];
            }
            vm->cf = 0;
            if (a == b)      { vm->cf = CMP_EQ; }
            else if (a <= b) { vm->cf = CMP_LT; }
            else if (a >= b) { vm->cf = CMP_GT; }
            else if (a < b)  { vm->cf = CMP_LQ; }
            else if (a > b)  { vm->cf = CMP_GQ; }
            else if (a != b) { vm->cf = CMP_NQ; }
        }; break;

        // jif
        case 0b10100: {
            word cmp = read_bits(&buffer, &read_bits_count, 3);
            word addr = read_number(&buffer, &read_bits_count);
            if ((cmp == CMP_NQ && vm->cf != CMP_EQ) || cmp == vm->cf) {
                vm->ip = addr;
                return 1;
            }
        }; break;

        default:
            printf("Reached unknown instruction with opcode: 0x%02x\n", opcode);
            dump_vm(*vm, vm->program_size, "instr_unknown.dump");
            exit(1);
    }
    size_t read_bytes_count = read_bits_count / 8;
    if (read_bits_count / 8.0 > (int)(read_bits_count / 8.0)) {
        read_bytes_count += 1;
    }
    vm->ip += read_bytes_count;
    return 1;
}

void push_in_stack(VM *vm, word value) {
    if (vm->stack_begging - vm->sp >= STACK_SIZE) {
        fprintf(stderr, "Stack overflow (vm dumped)\n");
        dump_vm(*vm, vm->program_size, "stackowerflow.dump");
        exit(1);
    }
    vm->memory[vm->sp--] = value >> 8;
    vm->memory[vm->sp--] = value & 0xff;
}

word pop_from_stack(VM *vm) {
    if (vm->sp >= vm->stack_begging) {
        fprintf(stderr, "Stack is empty (vm dumped)\n");
        dump_vm(*vm, vm->program_size, "stackisempty.dump");
        exit(1);
    }
    word value = vm->memory[++vm->sp];
    value |= vm->memory[++vm->sp];
    return value;
}

void call_function(VM *vm, word func_addr, word ret_addr) {
    push_in_stack(vm, ret_addr);
    vm->ip = func_addr;
}

void return_from_function(VM *vm) {
    vm->ip = pop_from_stack(vm);
}

// ------------------------------------------------------------------------------------------------

void dump_vm(VM vm, size_t memory_size_to_dump, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        error_file_doesnot_exist(filename);
    }

    fprintf(fp, "Registers:\n");
    for (size_t i = 0; i < 4; i++) {
        fprintf(fp, "r%lu: 0x%04x    ", i, vm.general_registers[i]);
        fprintf(fp, "r%lu: 0x%04x    ", i + 4, vm.general_registers[i + 4]);
        if (i + 8 <= 12) {
            fprintf(fp, "r%lu: 0x%04x", i + 8, vm.general_registers[i + 8]);
        }
        // Only for 13th
        if (i == 0) {
            fprintf(fp, "    r%d: 0x%04x", 12, vm.general_registers[12]);
        }
        fprintf(fp, "\n");
    }
    fprintf(fp, "--------\n");
    fprintf(fp, "ip: 0x%04x\n", vm.ip);
    fprintf(fp, "cf: 0x%04x\n", vm.cf);
    fprintf(fp, "sp: 0x%04x\n", vm.sp);

    fprintf(fp, "\n\nMemory");
    if (memory_size_to_dump == 0) {
        fprintf(fp, ":\nMemory was not dumped\n");
    } else {
        fprintf(fp, " (%lu bytes):\n", memory_size_to_dump);
    }
    for (size_t i = 0; i < memory_size_to_dump; i++) {
        if (i != 0 && i % 16 == 0) {
            fprintf(fp, "\n");
        }
        fprintf(fp, "%02x ", vm.memory[i]);
    }

    fprintf(fp, "\n\nStack\n");
    size_t counter = 0;
    for (byte *b = vm.memory + vm.stack_begging; b > vm.memory + vm.sp; b--, counter++) {
        if (counter != 0 && counter % 16 == 0) {
            fprintf(fp, "\n");
        }
        fprintf(fp, "%02x ", *b);
    }
    fprintf(fp, "\n");

    fclose(fp);
}
