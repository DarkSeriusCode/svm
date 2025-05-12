#include "device.h"
#include "io.h"
#include <dlfcn.h>
#include <string.h>

static void check_dl_error(void *dl) {
    const char *msg = dlerror();
    if (msg) {
        error_dl(dl, msg);
    }
}

Device new_device(const char *filename) {
    void *dl = dlopen(filename, RTLD_NOW);
    if (!dl)
        error_dl(dl, dlerror());
    InitFunc *init = dlsym(dl, "init");
    check_dl_error(dl);
    FiniFunc *fini = dlsym(dl, "fini");
    check_dl_error(dl);
    ReadFunc *read = dlsym(dl, "read");
    check_dl_error(dl);
    WriteFunc *write = dlsym(dl, "write");
    check_dl_error(dl);
    return (Device) { dl, filename, init, fini, read, write };
}

void free_device(void *dev) {
    dlclose(((Device *)dev)->dl);
    memset(dev, 0, sizeof(Device));
}
