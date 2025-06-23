#include "io.h"
#include <dlfcn.h>
#include <stdio.h>

extern const char *INPUT_FILE_NAME;

static void print_error(const char *msg, ...) {
    va_list args;
    va_start(args, msg);

    style(STYLE_BOLD);
    printf("%s", INPUT_FILE_NAME);
    printf(": ");
    printf_red("error: ");
    vprintf(msg, args);
    printf("\n");

    va_end(args);
}

void error_dl(void *dl, const char *msg) {
    print_error("Error while loading a device: \"%s\"", msg);
    if (dl)
        dlclose(dl);
    exit(EXIT_FAILURE);
}

void error_dev_open(const char *device_file, word code) {
    print_error("while opening %s (code: %d)", device_file, code);
    exit(EXIT_FAILURE);
}

void error_dev_close(const char *device_file, word code) {
    print_error("while closing %s (code: %d)", device_file, code);
    exit(EXIT_FAILURE);
}

void error_no_device_attached(word port_id) {
    print_error("no device connected to the port with id %d", port_id);
    exit(EXIT_FAILURE);
}

void error_no_free_ports(void) {
    print_error("cannot attach the device: no free ports");
    exit(EXIT_FAILURE);
}

void error_using_preserve_device(void) {
    print_error("device 0 is reserved");
    exit(EXIT_FAILURE);
}

void error_too_big_program(void) {
    print_error("program cannot be placed into memory (which is %d bytes)", MEMORY_SIZE);
    exit(EXIT_FAILURE);
}

void dump_vm(VM vm, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        error_file_doesnot_exist(filename);
    }

    fprintf(fp, "Devices:\n");
    if (vector_size(vm.ports) == 0) {
        fprintf(fp, "No connected devices\n");
    }
    foreach(Port, port, vm.ports) {
        fprintf(fp, "%s -> %d\n", port->device.filename, port->id);
    }

    fprintf(fp, "\nRegisters:\n");
    for (size_t i = 0; i < 4; i++) {
        fprintf(fp, "r%lu: 0x%04x    ", i, vm.registers[i]);
        fprintf(fp, "r%lu: 0x%04x    ", i + 4, vm.registers[i + 4]);
        if (i + 8 <= 12) {
            fprintf(fp, "r%lu: 0x%04x", i + 8, vm.registers[i + 8]);
        }
        // Only for 13th
        if (i == 0) {
            fprintf(fp, "    r%d: 0x%04x", 12, vm.registers[12]);
        }
        fprintf(fp, "\n");
    }
    fprintf(fp, "--------\n");
    fprintf(fp, "ip: 0x%04x\n", vm.registers[REG_IP]);
    fprintf(fp, "cf: 0x%04x\n", vm.registers[REG_CF]);
    fprintf(fp, "sp: 0x%04x\n", vm.registers[REG_SP]);

    fprintf(fp, "\nMemory");
    if (vm.program_size == 0) {
        fprintf(fp, ":\nMemory was not dumped\n");
    } else {
        fprintf(fp, " (%lu bytes):\n", vm.program_size);
    }
    for (size_t i = 0; i < vm.program_size; i++) {
        if (i != 0 && i % 16 == 0) {
            fprintf(fp, "\n");
        }
        fprintf(fp, "%02x ", vm.memory[i]);
    }

    fprintf(fp, "\n\nStack:\n");
    size_t counter = 0;
    for (byte *b = vm.memory + vm.stack_begging; b > vm.memory + vm.registers[REG_SP]; b--, counter++) {
        if (counter != 0 && counter % 16 == 0) {
            fprintf(fp, "\n");
        }
        fprintf(fp, "%02x ", *b);
    }
    fprintf(fp, "\n");

    fclose(fp);
}
