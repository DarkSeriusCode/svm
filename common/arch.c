#include "arch.h"
#include "utils.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>

const char *ZERO_OP_INSTRUCTIONS[] = { "ret" };
const char *ONE_OP_INSTRUCTIONS[] = { "push", "pop", "call", "not", "jmp" };
const char *TWO_OPS_INSTRUCTIONS[] = { "ld", "str", "mov", "add", "sub", "mul", "div",
                                       "and", "or", "xor", "shl", "shr", "cmp", "jif" };
const char *THREE_OPS_INSTRUCTIONS[] = { "out", "in" };
const char *DIRECTIVES[] = { "use" };
const char *REGISTER_SET[] = { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10",
                               "r11", "r12",  "sp", "ip", "cf" };

Cmp cmp_from_string(const char *string) {
    if (strcmp(string, "eq") == 0) return CMP_EQ;
    if (strcmp(string, "nq") == 0) return CMP_NQ;
    if (strcmp(string, "lt") == 0) return CMP_LT;
    if (strcmp(string, "lq") == 0) return CMP_LQ;
    if (strcmp(string, "gt") == 0) return CMP_GT;
    if (strcmp(string, "gq") == 0) return CMP_GQ;
    UNREACHABLE("If you see this, something actually went wrong, create an issue");
}

InstrOpcode instropcode_from_str(const char *string) {
    if (strcmp(string, "mov") == 0)  return INSTR_MOV;
    if (strcmp(string, "ld") == 0)   return INSTR_LD;
    if (strcmp(string, "str") == 0)   return INSTR_ST;
    if (strcmp(string, "add") == 0)  return INSTR_ADD;
    if (strcmp(string, "sub") == 0)  return INSTR_SUB;
    if (strcmp(string, "mul") == 0)  return INSTR_MUL;
    if (strcmp(string, "div") == 0)  return INSTR_DIV;
    if (strcmp(string, "not") == 0)  return INSTR_NOT;
    if (strcmp(string, "push") == 0) return INSTR_PUSH;
    if (strcmp(string, "pop") == 0)  return INSTR_POP;
    if (strcmp(string, "call") == 0) return INSTR_CALL;
    if (strcmp(string, "ret") == 0)  return INSTR_RET;
    if (strcmp(string, "and") == 0)  return INSTR_AND;
    if (strcmp(string, "or") == 0)   return INSTR_OR;
    if (strcmp(string, "xor") == 0)  return INSTR_XOR;
    if (strcmp(string, "shl") == 0)  return INSTR_SHL;
    if (strcmp(string, "shr") == 0)  return INSTR_SHR;
    if (strcmp(string, "jmp") == 0)  return INSTR_JMP;
    if (strcmp(string, "cmp") == 0)  return INSTR_CMP;
    if (strcmp(string, "jif") == 0)  return INSTR_JIF;
    if (strcmp(string, "out") == 0)  return INSTR_OUT;
    if (strcmp(string, "in") == 0)   return INSTR_IN;
    printf(string);
    UNREACHABLE("If you see this, something actually went wrong, create an issue");
}

bool in_instruction_set(const char *inst) {
    return in_zero_op_instruction_set(inst) || in_one_op_instruction_set(inst)
           || in_two_ops_instruction_set(inst) || in_three_ops_instruction_set(inst);
}

bool in_zero_op_instruction_set(const char *inst) {
    return string_in_array(inst, ZERO_OP_INSTRUCTIONS, ARRAY_LEN(ZERO_OP_INSTRUCTIONS));
}

bool in_one_op_instruction_set(const char *inst) {
    return string_in_array(inst, ONE_OP_INSTRUCTIONS, ARRAY_LEN(ONE_OP_INSTRUCTIONS));
}

bool in_two_ops_instruction_set(const char *inst) {
    return string_in_array(inst, TWO_OPS_INSTRUCTIONS, ARRAY_LEN(TWO_OPS_INSTRUCTIONS));
}

bool in_three_ops_instruction_set(const char *inst) {
    return string_in_array(inst, THREE_OPS_INSTRUCTIONS, ARRAY_LEN(THREE_OPS_INSTRUCTIONS));
}

bool in_register_set(const char *reg) {
    return string_in_array(reg, REGISTER_SET, ARRAY_LEN(REGISTER_SET));
}

bool in_directive_set(const char *name) {
    return string_in_array(name, DIRECTIVES, ARRAY_LEN(DIRECTIVES));
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

byte get_dir_param_count(const char *dir_name) {
    if (strcmp(dir_name, "use") == 0) return 2;
    return 0;
}

byte get_dir_code(const char *dir_name) {
    assert(in_directive_set(dir_name));
    if (strcmp(dir_name, "use") == 0) return 0b001;
    return -1;
}
