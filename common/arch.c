#include "arch.h"
#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

const char *INSTRUCTION_SET[] = { "load", "mov", "add", "sub", "store", "mul", "div" };
const char *TWO_OPS_INSTRUCTIONS[] = { "load", "store", "mov", "add", "sub", "mul", "div" };
const char *REGISTER_SET[] = { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "sp", "ip", "cf" };

bool in_instruction_set(const char *inst) {
    for (size_t i = 0; i < sizeof(INSTRUCTION_SET)/sizeof(INSTRUCTION_SET[0]); i++)
        if (strcmp(INSTRUCTION_SET[i], inst) == 0) return true;
    return false;
}

bool in_two_ops_instruction_set(const char *inst) {
    for (size_t i = 0; i < sizeof(TWO_OPS_INSTRUCTIONS)/sizeof(TWO_OPS_INSTRUCTIONS[0]); i++)
        if (strcmp(TWO_OPS_INSTRUCTIONS[i], inst) == 0) return true;
    return false;
}

bool in_register_set(const char *inst) {
    for (size_t i = 0; i < sizeof(REGISTER_SET)/sizeof(REGISTER_SET[0]); i++)
        if (strcmp(REGISTER_SET[i], inst) == 0) return true;
    return false;
}

byte get_instr_opcode(const char *instr_name) {
    byte opcode = 0;
    if (strcmp(instr_name, "mov") == 0) opcode = 0b0001;
    if (strcmp(instr_name, "load") == 0) opcode = 0b0010;
    if (strcmp(instr_name, "store") == 0) opcode = 0b0011;
    if (strcmp(instr_name, "add") == 0) opcode = 0b0100;
    if (strcmp(instr_name, "sub") == 0) opcode = 0b0101;
    if (strcmp(instr_name, "mul") == 0) opcode = 0b0110;
    if (strcmp(instr_name, "div") == 0) opcode = 0b0111;
    return opcode;
}

size_t get_instr_size(const char *instr_name) {
    if (strcmp(instr_name, "mov") == 0) return 4;
    if (strcmp(instr_name, "load") * strcmp(instr_name, "store") == 0) return 3;
    return 2;
}

byte get_register_code(const char *reg_name) {
    assert(in_register_set(reg_name));
    if (strcmp(reg_name, "r0") == 0) return 0b0000;
    if (strcmp(reg_name, "r1") == 0) return 0b0001;
    if (strcmp(reg_name, "r2") == 0) return 0b0010;
    if (strcmp(reg_name, "r3") == 0) return 0b0011;
    if (strcmp(reg_name, "r4") == 0) return 0b0100;
    if (strcmp(reg_name, "r5") == 0) return 0b0101;
    if (strcmp(reg_name, "r6") == 0) return 0b0110;
    if (strcmp(reg_name, "r7") == 0) return 0b0111;
    if (strcmp(reg_name, "sp") == 0) return 0b1000;
    if (strcmp(reg_name, "ip") == 0) return 0b1001;
    if (strcmp(reg_name, "cf") == 0) return 0b1010;
    return -1;
}
