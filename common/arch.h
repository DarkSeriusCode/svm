#ifndef __COMMON_ARCH_H
#define __COMMON_ARCH_H

#include <stdbool.h>
#include <stddef.h>

typedef unsigned char byte;
typedef unsigned short word;

#define OPCODE_BIT_SIZE 5
#define REGISTER_BIT_SIZE 4

#define ENTRY_POINT_NAME "_main"

bool in_instruction_set(const char *inst);
bool in_zero_op_instruction_set(const char *inst);
bool in_one_op_instruction_set(const char *inst);
bool in_two_ops_instruction_set(const char *inst);
bool in_register_set(const char *inst);

byte get_instr_opcode(const char *instr_name);
size_t get_instr_size(const char *instr_name);
byte get_register_code(const char *reg_name);

#endif
