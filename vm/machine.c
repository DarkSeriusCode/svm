#include "machine.h"
#include "common/error.h"
#include <string.h>
#include <stdio.h>

word read_word_as_big_endian(byte *memory) {
    word w = memory[0];
    w <<= 8;
    w |= memory[1];
    return w;
}

// ------------------------------------------------------------------------------------------------

VM new_vm(byte *memory, size_t program_size) {
    VM vm = {
        .program_size = program_size,
        .memory = memory,
        .cf = 0,
        .sp = program_size + STACK_OFFSET,
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
    printf("Opcode: 0x%02x\n", opcode);
    switch (opcode) {
        // loadb
        case 0b00010: {
            word buff = read_word_as_big_endian(mem);
            byte reg = (buff >> (16 - OPCODE_BIT_SIZE - REGISTER_BIT_SIZE)) & 0xf;
            word ident_addr = read_word_as_big_endian(mem + 2);
            vm->general_registers[reg] = vm->memory[ident_addr];
            vm->ip += get_instr_size("loadb");
        }; break;

        // mov
        case 0b00001: {
            word buff = read_word_as_big_endian(mem);
            byte reg = (buff >> (16 - OPCODE_BIT_SIZE - REGISTER_BIT_SIZE)) & 0xf;
            word num = read_word_as_big_endian(mem + 2);
            vm->general_registers[reg] += num;
            vm->ip += get_instr_size("mov");
        }; break;

        // add
        case 0b00100: {
            word buff = read_word_as_big_endian(mem);
            byte reg_dest = (buff >> (16 - OPCODE_BIT_SIZE - REGISTER_BIT_SIZE)) & 0xf;
            byte reg_src = (mem[1] >> (8 - 1 - REGISTER_BIT_SIZE)) & 0xf;
            vm->general_registers[reg_dest] += vm->general_registers[reg_src];
            vm->ip += get_instr_size("add");
        }; break;

        // storeb
        case 0b00011: {
            word buff = read_word_as_big_endian(mem);
            byte reg = (buff >> (16 - OPCODE_BIT_SIZE - REGISTER_BIT_SIZE)) & 0xf;
            word ident_addr = read_word_as_big_endian(mem + 2);
            vm->memory[ident_addr] = vm->general_registers[reg];
            vm->ip += get_instr_size("storeb");
        }; break;

        default:
            printf("Instruction with opcode 0x%02x is not supported yet\n", opcode);
            return 0;
    }
    return 1;
}

// ------------------------------------------------------------------------------------------------

void dump_vm(VM vm, size_t memory_size_to_dump, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        error_file_doesnot_exist(filename);
    }

    fprintf(fp, "Registers:\n");
    for (size_t i = 0; i < 4; i++) {
        fprintf(fp, "r%lu: 0x%04x    ", i, vm.general_registers[i]);
        fprintf(fp, "r%lu: 0x%04x\n", i + 4, vm.general_registers[i + 4]);
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

    fclose(fp);
}
