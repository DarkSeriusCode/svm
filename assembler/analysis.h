#ifndef __ASM_ANALYSIS_H
#define __ASM_ANALYSIS_H

#include "parser.h"
#include "common/vector.h"
#include "program.h"

void check_number_bounds(Token op, size_t should_has_size);
void check_decl_bounds(Decl decl);
void check_instr_op_bounds(Instr instr);
void check_names_existence(Instr instr, vector(Label) labels);

void analyse_label(Label label);
void analyse_directive(Directive dir);
void analyse_program(Program prog);

#endif
