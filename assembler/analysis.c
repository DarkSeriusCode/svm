#include "analysis.h"
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
    if (strcmp(instr.name, "load") * strcmp(instr.name, "store") * strcmp(instr.name, "movi") != 0)
        return;
    check_number_bounds(instr.ops[1], 2);
}

void check_data_label(Label label) {
    foreach(Decl, decl, label.declarations) {
        check_decl_bounds(*decl);
    }
}

void check_code_label(Label label) {
    foreach(Instr, instr, label.instructions) {
        check_instr_op_bounds(*instr);
    }
}

void analyse_program(vector(Label) labels) {
    // Analyse data labels
    foreach(Label, label, labels) {
        if (labels->is_empty) continue;
        if (label->is_data) check_data_label(*label);
        else check_code_label(*label);
    }
}
