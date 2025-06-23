#include "machine.h"
#include "common/io.h"
#include "common/arch.h"
#include "common/sex.h"
#include "io.h"
#include "common/str.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

Symbol new_symbol(const char *name, word declaration_address) {
    return (Symbol) { new_string(name), declaration_address };
}

void free_symbol(void *symbol) {
    Symbol symb = *(Symbol *)symbol;
    free_string(&symb.name);
}

word read_word_as_big_endian(byte *memory) {
    word w = memory[0];
    w <<= 8;
    w |= memory[1];
    return w;
}

unsigned long read_ulong_as_big_endian(byte *memory) {
    unsigned long buffer = memory[0];
    for (size_t i = 1; i < sizeof(unsigned long); i++) {
        buffer <<= 8;
        buffer |= memory[i];
    }
    return buffer;
}

static word read_bits(unsigned long *buffer, size_t *read_bits_count, size_t size) {
    word data = (*buffer >> (sizeof(unsigned long) - sizeof(word)) * 8);
    data >>= sizeof(word) * 8 - size;
    *buffer <<= size;
    *read_bits_count += size;
    return data;
}
#define read_opcode(buffer, read_count)    read_bits(buffer, read_count, OPCODE_BIT_SIZE)
#define read_register(buffer, read_count)  read_bits(buffer, read_count, REGISTER_BIT_SIZE)
#define read_flag(buffer, read_count)      read_bits(buffer, read_count, 1)
#define read_number(buffer, read_count)    read_bits(buffer, read_count, NUMBER_BIT_SIZE)
#define read_byte(buffer, read_count)      read_bits(buffer, read_count, 8)
#define skip_alignment(buffer, read_count, alignment)  read_bits(buffer, read_count, alignment)

// Perform binary operation in instructions like <reg> <reg/imm>
#define reg_reg_binop(buffer, read_bits_count, op) \
    do {\
        word src_reg = read_register(buffer, read_bits_count); \
        word flag = read_flag(buffer, read_bits_count); \
        short value; \
        if (flag) { \
            skip_alignment(buffer, read_bits_count, 6); \
            value = read_number(buffer, read_bits_count); \
        } else { \
            value = vm->registers[read_register(buffer, read_bits_count)]; \
        } \
        vm->registers[src_reg] op value; \
    } while(0);

// ------------------------------------------------------------------------------------------------

// Just a wraper around free_device that also calls dev.fini()
static void unload_port(void *port) {
    Device dev = ((Port *)port)->device;
    word code = dev.fini();
    if (code != 0) {
        error_dev_close(dev.filename, code);
    }
    free_device(&dev);
}

VM new_vm(const char *input_file) {
    ExecFile exec_file = execfile_read(input_file);
    byte *memory = malloc(MEMORY_SIZE);

    vector(Port) ports = NULL;
    vector_set_destructor(ports, unload_port);
    vector(Symbol) symbol_table = NULL;
    vector_set_destructor(symbol_table, free_symbol);

    VM vm = {
        .program_size = 0,
        .stack_begging = 0,
        .memory = memory,
        .ports = ports,
    };
    memset(vm.registers, 0, sizeof(vm.registers));

    vm_load_program_section(&vm, exec_file);
    vm_perform_directives(&vm, exec_file);
    vm_load_symbol_table(&vm, exec_file);

    foreach(Symbol, s, vm.symbol_table) {
        if (strcmp(s->name, ENTRY_POINT_NAME) == 0) {
            vm.registers[REG_IP] = s->declaration_address;
            break;
        }
    }

    return vm;
}

void free_vm(void *vm) {
    VM *v = (VM *)vm;
    free_vector(&v->ports);
    free_vector(&v->symbol_table);
    free(v->memory);
}

void vm_load_program_section(VM *vm, ExecFile exec_file) {
    vector(byte) compiled_program = execfile_get_section_content(exec_file, "program");
    if (compiled_program == NULL) {
        error_couldnot_find_section("program");
    }
    size_t program_size = vector_size(compiled_program);
    if (program_size > MEMORY_SIZE) {
        error_too_big_program();
    }
    memcpy(vm->memory, compiled_program, vector_size(compiled_program));
    vm->program_size = program_size;
    vm->stack_begging = program_size + STACK_OFFSET + STACK_MAX_SIZE;
    vm->registers[REG_SP] = vm->stack_begging;
    free_vector(&compiled_program);
    push_in_stack(vm, (word)program_size);
}

void vm_perform_directives(VM *vm, ExecFile exec_file) {
    vector(byte) compiled_directives = execfile_get_section_content(exec_file, "directives");
    if (!compiled_directives)
        return;
    byte *cursor = compiled_directives;
    byte *section_end = cursor + vector_size(compiled_directives) - 1;
    byte dir_code = *cursor++;
    while (cursor < section_end) {
        switch (dir_code) {
            case 0b001: {
                string path = NULL;
                while (*cursor != 0) string_push_char(&path, *cursor++);
                byte port = *(++cursor);
                vm_load_device(vm, path, port);
                free_string(&path);
            }; break;
        }
    }
    free_vector(&compiled_directives);
}

void vm_load_symbol_table(VM *vm, ExecFile exec_file) {
    vector(byte) compiled_symbols = execfile_get_section_content(exec_file, "symbols");
    if (compiled_symbols == NULL) {
        error_couldnot_find_section("symbols");
    }
    byte *cursor = compiled_symbols;
    byte *section_end = cursor + vector_size(compiled_symbols) - 1;
    string name = NULL;
    while (cursor < section_end) {
        while (*cursor != 0) string_push_char(&name, *cursor++);
        word addr = read_word_as_big_endian(++cursor);
        vector_push_back(vm->symbol_table, new_symbol(name, addr));
        cursor += 2;
        vector_clean(name);
    }
    free_vector(&compiled_symbols);
    free_string(&name);
}

void vm_load_device(VM *vm, const char *device_file, int port_id) {
    byte id = port_id;
    if (port_id == -1) {
        id = vm_get_free_port_id(*vm);
        printf("Device %s attached to port %d\n", device_file, id);
    }
    if (id == 0) {
        error_using_preserve_device();
    }
    Device dev = new_device(device_file);
    Port p = { id, dev };
    vector_push_back(vm->ports, p);
    word code = dev.init(vm->memory);
    if (code != 0) {
        error_dev_open(device_file, code);
    }
}

Port *vm_get_port(VM vm, byte port_id) {
    Port *port = NULL;
    foreach(Port, p, vm.ports) {
        if (p->id == port_id) {
            port = p;
            break;
        }
    }
    return port;
}

byte vm_get_free_port_id(VM vm) {
    for (word id = 1; id < 256; id++) {
        Port *p = vm_get_port(vm, id);
        if (!p) {
            return id;
        }
    }
    error_no_free_ports();
    return 0; // UNREACHABLE
}

int exec_instr(VM *vm) {
    if (vm->registers[REG_IP] >= vm->program_size) {
        return 0;
    }
    byte *mem = vm->memory + vm->registers[REG_IP];
    unsigned long buffer = read_ulong_as_big_endian(mem);
    size_t read_bits_count = 0;
    byte opcode = read_bits(&buffer, &read_bits_count, OPCODE_BIT_SIZE);
    switch (opcode) {
        // mov
        case 0b00001: {
            word dest_reg = read_register(&buffer, &read_bits_count);
            word flag = read_flag(&buffer, &read_bits_count);
            word value;
            if (flag) {
                skip_alignment(&buffer, &read_bits_count, 6);
                value = read_number(&buffer, &read_bits_count);
            } else {
                value = vm->registers[read_register(&buffer, &read_bits_count)];
            }
            vm->registers[dest_reg] = value;
        }; break;

        // load
        case 0b00010: {
            word dest_reg = read_register(&buffer, &read_bits_count);
            word flag = read_flag(&buffer, &read_bits_count);
            word addr;
            if (flag) {
                skip_alignment(&buffer, &read_bits_count, 6);
                addr = read_number(&buffer, &read_bits_count);
            } else {
                addr = vm->registers[read_register(&buffer, &read_bits_count)];
            }
            vm->registers[dest_reg] = read_word_as_big_endian(vm->memory + addr);
        }; break;

        // store
        case 0b00011: {
            word src_reg = read_register(&buffer, &read_bits_count);
            word flag = read_flag(&buffer, &read_bits_count);
            word addr;
            if (flag) {
                skip_alignment(&buffer, &read_bits_count, 6);
                addr = read_number(&buffer, &read_bits_count);
            } else {
                addr = vm->registers[read_register(&buffer, &read_bits_count)];
            }
            word value = vm->registers[src_reg];
            vm->memory[addr++] = value >> 8;
            vm->memory[addr] = value & 0xff;
        }; break;

        // add, sub, mul, div, and, or, xor, shl, shr
        case 0b00100: reg_reg_binop(&buffer, &read_bits_count, +=); break;
        case 0b00101: reg_reg_binop(&buffer, &read_bits_count, -=); break;
        case 0b00110: reg_reg_binop(&buffer, &read_bits_count, *=); break;
        case 0b00111: reg_reg_binop(&buffer, &read_bits_count, /=); break;
        case 0b01101: reg_reg_binop(&buffer, &read_bits_count, &=); break;
        case 0b01110: reg_reg_binop(&buffer, &read_bits_count, |=); break;
        case 0b01111: reg_reg_binop(&buffer, &read_bits_count, ^=); break;
        case 0b10000: reg_reg_binop(&buffer, &read_bits_count, <<=); break;
        case 0b10001: reg_reg_binop(&buffer, &read_bits_count, >>=); break;

        // not
        case 0b01000: {
            word reg = read_register(&buffer, &read_bits_count);
            vm->registers[reg] = ~vm->registers[reg];
        }; break;

        // push
        case 0b01001: {
            word reg = read_register(&buffer, &read_bits_count);
            push_in_stack(vm, vm->registers[reg]);
        }; break;

        // pop
        case 0b01010: {
            word reg = read_register(&buffer, &read_bits_count);
            word value = pop_from_stack(vm);
            vm->registers[reg] = value;
        }; break;

        // call
        case 0b01011: {
            skip_alignment(&buffer, &read_bits_count, 3);
            word func_addr = read_number(&buffer, &read_bits_count);
            word ret_addr = vm->registers[REG_IP] + 3;
            push_in_stack(vm, ret_addr);
            vm->registers[REG_IP] = func_addr;
            return 1;
        }; break;

        // ret
        case 0b01100: {
            vm->registers[REG_IP] = pop_from_stack(vm);
            return 1;
        }; break;

        // jmp
        case 0b10010: {
            skip_alignment(&buffer, &read_bits_count, 3);
            word addr = read_number(&buffer, &read_bits_count);
            vm->registers[REG_IP] = addr;
            return 1;
        }; break;

        // cmp
        case 0b10011: {
            word a = vm->registers[read_register(&buffer, &read_bits_count)];
            word flag = read_flag(&buffer, &read_bits_count);
            short b;
            if (flag) {
                skip_alignment(&buffer, &read_bits_count, 6);
                b = read_number(&buffer, &read_bits_count);
            } else {
                b = vm->registers[read_register(&buffer, &read_bits_count)];
            }
            vm->registers[REG_CF] = 0;
            if (a == b)      { vm->registers[REG_CF] = CMP_EQ; }
            else if (a <= b) { vm->registers[REG_CF] = CMP_LT; }
            else if (a >= b) { vm->registers[REG_CF] = CMP_GT; }
            else if (a < b)  { vm->registers[REG_CF] = CMP_LQ; }
            else if (a > b)  { vm->registers[REG_CF] = CMP_GQ; }
            else if (a != b) { vm->registers[REG_CF] = CMP_NQ; }
        }; break;

        // jif
        case 0b10100: {
            word cmp = read_bits(&buffer, &read_bits_count, 3);
            word addr = read_number(&buffer, &read_bits_count);
            if ((cmp == CMP_NQ && vm->registers[REG_CF] != CMP_EQ) || cmp == vm->registers[REG_CF]) {
                vm->registers[REG_IP] = addr;
                return 1;
            }
        }; break;

        // out
        case 0b10101: {
            byte port_id = read_byte(&buffer, &read_bits_count);
            bool is_first_num = read_flag(&buffer, &read_bits_count);
            bool is_second_num = read_flag(&buffer, &read_bits_count);
            word addr;
            if (is_first_num) {
                skip_alignment(&buffer, &read_bits_count, 1);
                addr = read_number(&buffer, &read_bits_count);
            } else {
                addr = vm->registers[read_register(&buffer, &read_bits_count)];
            }
            word size;
            if (is_second_num) {
                skip_alignment(&buffer, &read_bits_count, 1);
                size = read_number(&buffer, &read_bits_count);
            } else {
                size = vm->registers[read_register(&buffer, &read_bits_count)];
            }
            Port *port = vm_get_port(*vm, port_id);
            if (!port)
                error_no_device_attached(port_id);
            word code = port->device.write(addr, size);
            vm->registers[0] = code;
        }; break;

        // in
        case 0b10110: {
            byte port_id = read_byte(&buffer, &read_bits_count);
            bool is_first_num = read_flag(&buffer, &read_bits_count);
            bool is_second_num = read_flag(&buffer, &read_bits_count);
            word addr;
            if (is_first_num) {
                skip_alignment(&buffer, &read_bits_count, 1);
                addr = read_number(&buffer, &read_bits_count);
            } else {
                addr = vm->registers[read_register(&buffer, &read_bits_count)];
            }
            word size;
            if (is_second_num) {
                skip_alignment(&buffer, &read_bits_count, 1);
                size = read_number(&buffer, &read_bits_count);
            } else {
                size = vm->registers[read_register(&buffer, &read_bits_count)];
            }
            Port *port = vm_get_port(*vm, port_id);
            if (!port)
                error_no_device_attached(port_id);
            word code = port->device.read(addr, size);
            vm->registers[0] = code;
        }; break;

        default:
            printf("Reached unknown instruction with opcode: 0x%02x\n", opcode);
            dump_vm(*vm, "instr_unknown.dump");
            exit(EXIT_FAILURE);
    }
    size_t read_bytes_count = read_bits_count / 8;
    if (read_bits_count / 8.0 > (int)(read_bits_count / 8.0)) {
        read_bytes_count += 1;
    }
    vm->registers[REG_IP] += read_bytes_count;
    return 1;
}

void push_in_stack(VM *vm, word value) {
    if (vm->stack_begging - vm->registers[REG_SP] >= STACK_MAX_SIZE) {
        fprintf(stderr, "Stack overflow (vm dumped)\n");
        dump_vm(*vm, "stackowerflow.dump");
        exit(1);
    }
    vm->memory[vm->registers[REG_SP]--] = value >> 8;
    vm->memory[vm->registers[REG_SP]--] = value & 0xff;
}

word pop_from_stack(VM *vm) {
    if (vm->registers[REG_SP] >= vm->stack_begging) {
        fprintf(stderr, "Stack is empty (vm dumped)\n");
        dump_vm(*vm, "stackisempty.dump");
        exit(1);
    }
    word value = vm->memory[++vm->registers[REG_SP]];
    value |= vm->memory[++vm->registers[REG_SP]];
    return value;
}
