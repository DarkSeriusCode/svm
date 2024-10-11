#ifndef __COMMON_IO_H
#define __COMMON_IO_H

// Returns file content. Needs to be freed
char *read_whole_file(const char *filename);

void exit_with_erorr(const char *error_msg, ...);

#endif
