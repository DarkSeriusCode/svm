#ifndef __COMMON_ARCH_H
#define __COMMON_ARCH_H

#include <stdbool.h>
#include <stddef.h>

typedef unsigned char byte;
typedef unsigned short word;

#define OPCODE_BIT_SIZE 5
#define REGISTER_BIT_SIZE 4
#define NUMBER_BIT_SIZE sizeof(word) * 8

#define ENTRY_POINT_NAME "_main"

// a == b -> 001 (EQ)
// a != b -> 000 (NQ)
// a < b  -> 100 (LT)
// a <= b -> 101 (LQ)
// a > b  -> 110 (GT)
// a >= b -> 111 (GQ)
typedef enum {
    CMP_EQ = 0b001,
    CMP_NQ = 0b000,
    CMP_LT = 0b100,
    CMP_LQ = 0b101,
    CMP_GT = 0b110,
    CMP_GQ = 0b111,
} Cmp;
Cmp cmp_from_string(const char *string);

typedef enum {
    INSTR_MOV  = 0b00001,
    INSTR_LD,
    INSTR_ST,
    INSTR_ADD,
    INSTR_SUB,
    INSTR_MUL,
    INSTR_DIV,
    INSTR_NOT,
    INSTR_PUSH,
    INSTR_POP,
    INSTR_CALL,
    INSTR_RET,
    INSTR_AND,
    INSTR_OR,
    INSTR_XOR,
    INSTR_SHL,
    INSTR_SHR,
    INSTR_JMP,
    INSTR_CMP,
    INSTR_JIF,
    INSTR_OUT,
    INSTR_IN,
    INSTR_COUNT,
} InstrOpcode;
InstrOpcode instropcode_from_str(const char *string);

bool in_instruction_set(const char *inst);
bool in_zero_op_instruction_set(const char *inst);
bool in_one_op_instruction_set(const char *inst);
bool in_two_ops_instruction_set(const char *inst);
bool in_three_ops_instruction_set(const char *inst);
bool in_register_set(const char *reg);
bool in_directive_set(const char *dir);

byte get_register_code(const char *reg_name);
byte get_dir_param_count(const char *dir_name);
byte get_dir_code(const char *dir_name);

#endif
