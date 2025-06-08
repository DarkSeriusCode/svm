#ifndef __STR_H
#define __STR_H

#ifdef STR_IMPLEMENTATION
#define VECTOR_IMPLEMENTATION
#endif

#include "vector.h"

typedef vector(char) string;

string new_string(const char *str);
void string_push_char(string *s, char ch);

#define free_string(str) free_vector(str)
#define string_clean(str) vector_clean(str)

#ifdef STR_IMPLEMENTATION

#include <string.h>

string new_string(const char *str) {
    string s = NULL;
    vector_reserve(s, strlen(str) + 1);
    strcpy(s, str);
    __vector_set_size(s, strlen(str) + 1);
    return s;
}

void string_push_char(string *s, char ch) {
    if (*s != NULL && !vector_empty(*s)) {
        vector_erase(*s, strlen(*s));
    }
    vector_push_back(*s, ch);
    vector_push_back(*s, 0);
}

#endif

#endif
