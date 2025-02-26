#ifndef __ASM_ANALYSIS_H
#define __ASM_ANALYSIS_H

#include "parser.h"
#include "common/vector.h"

void check_number_bounds(Token op, size_t should_has_size);
void check_decl_bounds(Decl decl);
void check_instr_op_bounds(Instr instr);
void check_names_existence(Instr instr, vector(Label) labels);

void check_data_label(Label label);
void check_code_label(Label label);

void analyse_program(vector(Label) labels);

#endif
