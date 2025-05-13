#ifndef __VM_IO_H
#define __VM_IO_H

#include "common/io.h"
#include "machine.h"

void error_dl(void *dl, const char *msg);
void error_dev_open(const char *device_file, word status);
void error_dev_close(const char *device_file, word status);
void error_no_device_attached(word port_id);
void error_no_free_ports(void);
void error_using_preserve_device(void);

void dump_vm(VM vm, const char *filename);

#endif
