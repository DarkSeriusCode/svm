#define VECTOR_IMPLEMENTATION

#include "common/io.h"
#include "io.h"
#include "common/vector.h"
#include "vm/machine.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

const char *INPUT_FILE_NAME;
bool ENABLE_COLORS = true;

void print_help(const char *name);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_help(argv[0]);
        return 1;
    }

    vector(char *) device_files = NULL;
    int res = 0;
    while ( (res = getopt(argc, argv, "hd:")) != -1 ) {
        switch (res) {
            case 'h':
                print_help(argv[0]);
                exit(0);
            case 'd':
                vector_push_back(device_files, optarg);
                break;
        }
    }
    INPUT_FILE_NAME = argv[optind];
    if (!INPUT_FILE_NAME) {
        fprintf(stderr, "You have to provide a program file!\n");
        return 1;
    }

    VM vm = new_vm(INPUT_FILE_NAME);
    foreach(char *, device_file, device_files) {
        vm_load_device(&vm, *device_file);
    }
    while (exec_instr(&vm)) {}

    dump_vm(vm, "vm.dump");

    free_vm(&vm);
    free_vector(&device_files);

    return 0;
}

void print_help(const char *name) {
    printf("%s - a simple virtual machine.\n", name);
    printf("Usage: %s [options] program_file\n", name);
    printf("Options:\n");
    printf("  -h          Prints this message and exit\n");
    printf("  -d <file>   Loads device\n");
}
