#include "common/arch.h"
#include "common/io.h"
#include "vm/machine.h"
#include <stdio.h>
#include <string.h>

const size_t MEMORY_SIZE = 256 * 256;

const char *INPUT_FILE_NAME;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <program_image>\n", argv[0]);
        return 1;
    }
    INPUT_FILE_NAME = argv[1];
    byte memory[MEMORY_SIZE];
    memset(memory, 0, sizeof(memory));
    size_t program_size = load_program(memory, MEMORY_SIZE, INPUT_FILE_NAME);

    VM vm = new_vm(memory, program_size);
    dump_vm(vm, 8, "vm1.dump");
    while (exec_instr(&vm)) {}

    dump_vm(vm, 8, "vm2.dump");

    return 0;
}
