#include "error.h"
#include <stdio.h>
#include <stdlib.h>

void error_file_doesnot_exist(const char *filename) {
    printf("File not found: %s\n", filename);
    exit(EXIT_FAILURE);
}
