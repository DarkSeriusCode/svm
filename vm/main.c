#define VECTOR_IMPLEMENTATION

#include "common/io.h"
#include "io.h"
#include "common/vector.h"
#include "vm/machine.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

// I don't know how to name it better, so it is what it is
typedef struct {
    const char *device_file;
    int port_id; // May be negative. -1 means free port
} DeviceFileAndPort;

const char *INPUT_FILE_NAME;
bool ENABLE_COLORS = true;

void print_help(const char *name);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_help(argv[0]);
        return 1;
    }

    vector(DeviceFileAndPort) devices_to_attach = NULL;
    int res = 0;
    while ( (res = getopt(argc, argv, "hd:")) != -1 ) {
        switch (res) {
            case 'h':
                print_help(argv[0]);
                exit(0);
            case 'd': {
                char *unused;
                char *file = strtok(optarg, ":");
                char *port = strtok(NULL, ":");
                int port_id = -1;
                if (port)
                    port_id = strtol(port, &unused, 10);
                DeviceFileAndPort p = (DeviceFileAndPort){ file, port_id };
                vector_push_back(devices_to_attach, p);
            }; break;
        }
    }
    INPUT_FILE_NAME = argv[optind];
    if (!INPUT_FILE_NAME) {
        fprintf(stderr, "You have to provide a program file!\n");
        return 1;
    }

    VM vm = new_vm(INPUT_FILE_NAME);
    foreach(DeviceFileAndPort, port, devices_to_attach) {
        vm_load_device(&vm, port->device_file, port->port_id);
    }
    while (exec_instr(&vm)) {}

    dump_vm(vm, "vm.dump");

    free_vm(&vm);
    free_vector(&devices_to_attach);

    return 0;
}

void print_help(const char *name) {
    printf("%s - a simple virtual machine.\n", name);
    printf("Usage: %s [options] program_file\n", name);
    printf("Options:\n");
    printf("  -h          Prints this message and exit\n");
    printf("  -d <file>   Loads device\n");
}
