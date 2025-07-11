#include <common/arch.h>
#include <common/utils.h>
#include <stdio.h>
#include <string.h>

static byte *memory = NULL;

word init(byte *mem) {
    memory = mem;
    return 0;
}

word fini(void) { return 0; }

word read(word buffer_addr, word buffer_size) {
    char buff[buffer_size];
    fgets(buff, buffer_size, stdin);
    memcpy(memory + buffer_addr, buff, buffer_size);
    return strlen(buff);
}

word write(word buffer_addr, word buffer_size) {
    printf("%.*s", buffer_size, memory + buffer_addr);
    return buffer_size;
}
