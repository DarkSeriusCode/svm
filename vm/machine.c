#include "machine.h"
#include "common/io.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define print_op(op) printf("    "#op": 0x%02x (%d)\n", op, op)

word read_word_as_big_endian(byte *memory) {
    word w = memory[0];
    w <<= 8;
    w |= memory[1];
    return w;
}

// Reads two register ops in commands like <opcode> <reg> <reg>
static void read_two_regs(byte *mem, byte *reg1, byte *reg2) {
    word buffer = 0;
    buffer |= (mem[0] & 0x7) << 1;
    buffer |= mem[1] >> 7;
    *reg1 = buffer; buffer = 0;
    buffer |= (mem[1] & 0x78) >> 3;
    *reg2 = buffer;
}

// Reads two registers via read_two_regs and performs an action on them, saving result in the first
// register
#define two_reg_operation(vm, mem, op) \
    do { \
        byte _reg1 = 0, _reg2 = 0; \
        read_two_regs(mem, &_reg1, &_reg2); \
        vm->general_registers[_reg1] op vm->general_registers[_reg2]; \
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

    return vm;
}

int exec_instr(VM *vm) {
    if (vm->ip >= vm->program_size) {
        return 0;
    }
    byte *mem = vm->memory + vm->ip;
    byte opcode = *mem >> (8 - OPCODE_BIT_SIZE);
    word buffer = 0;
    switch (opcode) {
        // load
        case 0b00010: {
            buffer |= (mem[0] & 0x7) << 1;
            buffer |= mem[1] >> 7;
            byte dest_reg = buffer;
            buffer = (mem[1] >> 6) & 1;
            byte is_reg_bit = buffer; buffer = 0;
            if (is_reg_bit) {
                buffer |= (mem[1] >> 2) & 0xF;
                byte src_reg = buffer;
                word address = vm->general_registers[src_reg];
                vm->general_registers[dest_reg] = read_word_as_big_endian(vm->memory + address);
            } else {
                buffer |= mem[2] << 8;
                buffer |= mem[3];
                word address = buffer;
                vm->general_registers[dest_reg] = read_word_as_big_endian(vm->memory + address);
            }
            vm->ip += get_instr_size("load");
        }; break;

        // movi
        case 0b01000: {
            buffer |= (mem[0] & 0x7) << 1;
            buffer |= mem[1] >> 7;
            byte dest_reg = buffer; buffer = 0;
            buffer |= mem[2] << 8;
            buffer |= mem[3];
            word num = buffer;
            vm->general_registers[dest_reg] = num;
            vm->ip += get_instr_size("movi");
        }; break;

        // mov
        case 0b00001: {
            two_reg_operation(vm, mem, =);
            vm->ip += get_instr_size("mov");
        }; break;

        // add
        case 0b00100: {
            two_reg_operation(vm, mem, +=);
            vm->ip += get_instr_size("add");
        }; break;

        // sub
        case 0b00101: {
            two_reg_operation(vm, mem, -=);
            vm->ip += get_instr_size("sub");
        }; break;

        // mul
        case 0b00110: {
            two_reg_operation(vm, mem, *=);
            vm->ip += get_instr_size("mul");
        }; break;

        // div
        case 0b00111: {
            two_reg_operation(vm, mem, /=);
            vm->ip += get_instr_size("div");
        }; break;

        // store
        case 0b00011: {
            buffer |= (mem[0] & 0x7) << 1;
            buffer |= mem[1] >> 7;
            byte src_reg = buffer;
            word src_reg_value = vm->general_registers[src_reg];
            buffer = (mem[1] >> 6) & 1;
            byte is_reg_bit = buffer; buffer = 0;
            word address;
            if (is_reg_bit) {
                buffer |= (mem[1] >> 2) & 0xF;
                byte dest_reg = buffer;
                address = vm->general_registers[dest_reg];
            } else {
                buffer |= mem[2] << 8;
                buffer |= mem[3];
                address = buffer;
            }
            vm->memory[address++] = src_reg_value >> 8;
            vm->memory[address] = (byte)src_reg_value;
            vm->ip += get_instr_size("store");
        }; break;

        // push
        case 0b01001: {
            buffer |= (mem[0] & 0x7) << 1;
            buffer |= mem[1] >> 7;
            byte reg = buffer;
            word reg_value = vm->general_registers[reg];
            push_in_stack(vm, reg_value);
            vm->ip += get_instr_size("push");
        }; break;

        // pop
        case 0b01010: {
            buffer |= (mem[0] & 0x7) << 1;
            buffer |= mem[1] >> 7;
            byte reg = buffer;
            word stack_value = pop_from_stack(vm);
            vm->general_registers[reg] = stack_value;
            vm->ip += get_instr_size("pop");
        }; break;

        // and
        case 0b01101: {
            two_reg_operation(vm, mem, &=);
            vm->ip += get_instr_size("and");
        }; break;

        // or
        case 0b01110: {
            two_reg_operation(vm, mem, |=);
            vm->ip += get_instr_size("or");
        }; break;

        // xor
        case 0b01111: {
            two_reg_operation(vm, mem, ^=);
            vm->ip += get_instr_size("xor");
        }; break;

        // shl
        case 0b10000: {
            two_reg_operation(vm, mem, <<=);
            vm->ip += get_instr_size("shl");
        }; break;

        // shr
        case 0b10001: {
            two_reg_operation(vm, mem, >>=);
            vm->ip += get_instr_size("shr");
        }; break;

        // not
        case 0b10010: {
            byte reg = 0;
            reg |= (mem[0] & 0x7) << 1;
            reg |= mem[1] >> 7;
            vm->general_registers[reg] = ~vm->general_registers[reg];
            vm->ip += get_instr_size("not");
        }; break;

        // call
        case 0b01011: {
            word func_addr = read_word_as_big_endian(&mem[1]);
            word ret_addr = vm->ip += get_instr_size("call");
            call_function(vm, func_addr, ret_addr);
        }; break;

        // ret
        case 0b01100: {
            return_from_function(vm);
        }; break;

        default:
            printf("\nInstruction with opcode 0x%02x is not supported yet\n", opcode);
            exit(1);
    }
    return 1;
}

void push_in_stack(VM *vm, word value) {
    if (vm->stack_begging - vm->sp >= STACK_SIZE) {
        printf("Stack overflow (vm dumped)\n");
        dump_vm(*vm, vm->program_size, "stackowerflow.dump");
        exit(1);
    }
    vm->memory[vm->sp--] = value >> 8;
    vm->memory[vm->sp--] = value & 0xff;
}

word pop_from_stack(VM *vm) {
    word value = 0;
    value |= vm->memory[++vm->sp] << 8;
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
