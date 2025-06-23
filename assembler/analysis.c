#include "analysis.h"
#include "common/utils.h"
#include "io.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>

void check_number_bounds(Token op, size_t should_has_size) {
    assert(should_has_size == 1 || should_has_size == 2);
    char *strend;
    long n = strtol(op.value, &strend, 10);
    long lower_bound = 0, upper_bound = 0;
    if (should_has_size == 1) {
        if (n < 0) {
            lower_bound = CHAR_MIN;
            upper_bound = CHAR_MAX;
        } else {
            lower_bound = 0;
            upper_bound = UCHAR_MAX;
        }
    } else {
        if (n < 0) {
            lower_bound = SHRT_MIN;
            upper_bound = SHRT_MAX;
        } else {
            lower_bound = 0;
            upper_bound = USHRT_MAX;
        }
    }
    if (!(lower_bound <= n && n <= upper_bound)) {
        warning_number_out_of_bounds(n, lower_bound, upper_bound, op.span);
    }
}

void check_decl_bounds(Decl decl) {
    char *UNUSED;

    long value = strtol(decl.value.value, &UNUSED, 10);
    if (strcmp(decl.kind.value, ".ascii") == 0) return;

    if (strcmp(decl.kind.value, ".align") == 0) {
        if (value < 0) error_negative_alignment_size(decl.span);
        else if (value == 0) note_zero_alignment(decl.span);

    }
    if (strcmp(decl.kind.value, ".byte") == 0)
        check_number_bounds(decl.value, 1);
    if (strcmp(decl.kind.value, ".word") == 0)
        check_number_bounds(decl.value, 2);
}

void check_instr_op_bounds(Instr instr) {
    if (strcmp(instr.name, "call") == 0) return;

    if (string_in_args(instr.name, 2, "in", "out")) {
        check_number_bounds(instr.ops[0], 1);
    } else if (vector_size(instr.ops) == 2 && instr.ops[1].type != TOKEN_REG){
        check_number_bounds(instr.ops[1], 2);
    }
}

void analyse_directive(Directive dir) {
    if (strcmp(dir.name, "use") == 0) {
        check_number_bounds(dir.params[1], 1);
    }
}

void analyse_label(Label label) {
    if (label.is_data) {
        foreach(Decl, decl, label.declarations) {
            check_decl_bounds(*decl);
        }
    } else if (label.is_data && strcmp(label.name, ENTRY_POINT_NAME) == 0) {
        error_entry_point_with_decls();
    } else {
        foreach(Instr, instr, label.instructions) {
            check_instr_op_bounds(*instr);
        }
    }
}
