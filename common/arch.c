#include "arch.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

const char *ZERO_OP_INSTRUCTIONS[] = { "ret" };
const char *ONE_OP_INSTRUCTIONS[] = { "push", "pop", "call", "not", "jmp" };
const char *TWO_OPS_INSTRUCTIONS[] = { "load", "store", "mov", "add", "sub", "mul", "div",
                                       "and", "or", "xor", "shl", "shr", "cmp", "jif" };
const char *THREE_OPS_INSTRUCTIONS[] = { "out", "in" };
const char *REGISTER_SET[] = { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10",
                               "r11", "r12",  "sp", "ip", "cf" };

Cmp cmp_from_string(const char *string) {
    if (strcmp(string, "eq") == 0) return CMP_EQ;
    if (strcmp(string, "nq") == 0) return CMP_NQ;
    if (strcmp(string, "lt") == 0) return CMP_LT;
    if (strcmp(string, "lq") == 0) return CMP_LQ;
    if (strcmp(string, "gt") == 0) return CMP_GT;
    if (strcmp(string, "gq") == 0) return CMP_GQ;
    assert(0 == "Unreachable");
}

bool in_instruction_set(const char *inst) {
    return in_zero_op_instruction_set(inst) || in_one_op_instruction_set(inst)
           || in_two_ops_instruction_set(inst) || in_three_ops_instruction_set(inst);
}

bool in_zero_op_instruction_set(const char *inst) {
    for (size_t i = 0; i < sizeof(ZERO_OP_INSTRUCTIONS)/sizeof(ZERO_OP_INSTRUCTIONS[0]); i++)
        if (strcmp(ZERO_OP_INSTRUCTIONS[i], inst) == 0) return true;
    return false;
}

bool in_one_op_instruction_set(const char *inst) {
    for (size_t i = 0; i < sizeof(ONE_OP_INSTRUCTIONS)/sizeof(ONE_OP_INSTRUCTIONS[0]); i++)
        if (strcmp(ONE_OP_INSTRUCTIONS[i], inst) == 0) return true;
    return false;
}

bool in_two_ops_instruction_set(const char *inst) {
    for (size_t i = 0; i < sizeof(TWO_OPS_INSTRUCTIONS)/sizeof(TWO_OPS_INSTRUCTIONS[0]); i++)
        if (strcmp(TWO_OPS_INSTRUCTIONS[i], inst) == 0) return true;
    return false;
}

bool in_three_ops_instruction_set(const char *inst) {
    for (size_t i = 0; i < sizeof(THREE_OPS_INSTRUCTIONS)/sizeof(THREE_OPS_INSTRUCTIONS[0]); i++)
        if (strcmp(THREE_OPS_INSTRUCTIONS[i], inst) == 0) return true;
    return false;
}

bool in_register_set(const char *inst) {
    for (size_t i = 0; i < sizeof(REGISTER_SET)/sizeof(REGISTER_SET[0]); i++)
        if (strcmp(REGISTER_SET[i], inst) == 0) return true;
    return false;
}

byte get_instr_opcode(const char *instr_name) {
    byte opcode = 0;
    if (strcmp(instr_name, "mov") == 0) opcode = 0b00001;
    if (strcmp(instr_name, "load") == 0) opcode = 0b00010;
    if (strcmp(instr_name, "store") == 0) opcode = 0b00011;
    if (strcmp(instr_name, "add") == 0) opcode = 0b00100;
    if (strcmp(instr_name, "sub") == 0) opcode = 0b00101;
    if (strcmp(instr_name, "mul") == 0) opcode = 0b00110;
    if (strcmp(instr_name, "div") == 0) opcode = 0b00111;
    if (strcmp(instr_name, "not") == 0) opcode = 0b01000;
    if (strcmp(instr_name, "push") == 0) opcode = 0b01001;
    if (strcmp(instr_name, "pop") == 0) opcode = 0b01010;
    if (strcmp(instr_name, "call") == 0) opcode = 0b01011;
    if (strcmp(instr_name, "ret") == 0) opcode = 0b01100;
    if (strcmp(instr_name, "and") == 0) opcode = 0b01101;
    if (strcmp(instr_name, "or") == 0) opcode = 0b01110;
    if (strcmp(instr_name, "xor") == 0) opcode = 0b01111;
    if (strcmp(instr_name, "shl") == 0) opcode = 0b10000;
    if (strcmp(instr_name, "shr") == 0) opcode = 0b10001;
    if (strcmp(instr_name, "jmp") == 0) opcode = 0b10010;
    if (strcmp(instr_name, "cmp") == 0) opcode = 0b10011;
    if (strcmp(instr_name, "jif") == 0) opcode = 0b10100;
    if (strcmp(instr_name, "out") == 0) opcode = 0b10101;
    if (strcmp(instr_name, "in") == 0) opcode = 0b10110;
    return opcode;
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
    if (strcmp(reg_name, "r8") == 0) return 0b1000;
    if (strcmp(reg_name, "r9") == 0) return 0b1001;
    if (strcmp(reg_name, "r10") == 0) return 0b1010;
    if (strcmp(reg_name, "r11") == 0) return 0b1011;
    if (strcmp(reg_name, "r12") == 0) return 0b1100;
    if (strcmp(reg_name, "sp") == 0) return 0b1101;
    if (strcmp(reg_name, "ip") == 0) return 0b1110;
    if (strcmp(reg_name, "cf") == 0) return 0b1111;
    return -1;
}
