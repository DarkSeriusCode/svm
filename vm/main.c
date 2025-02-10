#include "common/arch.h"
#include "common/io.h"
#include "vm/machine.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const size_t MEMORY_SIZE = 256 * 256;

const char *INPUT_FILE_NAME;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <program_image>\n", argv[0]);
        return 1;
    }
    INPUT_FILE_NAME = argv[1];
    byte *memory = malloc(MEMORY_SIZE);
    memset(memory, 0, MEMORY_SIZE);
    size_t program_size = load_program(memory, MEMORY_SIZE, INPUT_FILE_NAME);

    VM vm = new_vm(memory, program_size);
    while (exec_instr(&vm)) {}

    dump_vm(vm, program_size, "vm.dump");
    free(memory);

    return 0;
}
