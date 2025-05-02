#ifndef __COMMON_VECTOR_H
#define __COMMON_VECTOR_H

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

typedef void (*VecElementDestructor)(void *);

typedef struct {
    size_t size;
    size_t capacity;
    size_t item_size;
    VecElementDestructor destructor;
} VecHeader;

#define vector(type) type*

#define vector_create_header(vec) \
    do { \
        if (!(vec)) { \
            vec = malloc(sizeof(VecHeader)); \
            VecHeader *hdr = (VecHeader *)vec; \
            hdr->size = hdr->capacity = 0; \
            hdr->item_size = sizeof(*(vec)); \
            hdr->destructor = NULL; \
            vec = vector_header_to_base(vec); \
        } \
    } while(0);

#define vector_grow(vec, new_cap) \
    do { \
        size_t vec_cap = vector_capacity(vec); \
        if (!(vec) && vec_cap == 0) { \
            vector_create_header(vec); \
        } \
        vec = (void *)vector_base_to_header(vec); \
        vec = realloc(vec, sizeof(*(vec)) * new_cap + sizeof(VecHeader)); \
        ((VecHeader *)vec)->capacity = new_cap; \
        vec = vector_header_to_base(vec); \
    } while(0);

#define vector_base_to_header(vec) \
    ( &(((VecHeader *)(vec)))[-1] )

#define vector_header_to_base(ptr) \
    ( (void *)&(((VecHeader *)ptr))[1] )

#define vector_capacity(vec) \
    ( (vec) ? vector_base_to_header(vec)->capacity : (size_t)0 )

#define vector_size(vec) \
    ( (vec) ? vector_base_to_header((vec))->size : (size_t)0 )

#define vector_item_size(vec) \
    ( (vec) ? vector_base_to_header(vec)->item_size : (size_t)0 )

// ONLY FOR INTERNAL USE.
#define __vector_set_size(vec, new_size) \
    (((VecHeader *)vector_base_to_header(vec))->size = (new_size))

// ONLY FOR INTERNAL USE
#define __vector_set_capacity(vec, new_cap) \
    (((VecHeader *)vector_base_to_header(vec))->capacity = (new_cap))

// ONLY FOR INTERNAL USE
#define __vector_free_item(vec, idx) \
    do { \
        VecElementDestructor destructor = ((VecHeader *)vector_base_to_header(vec))->destructor; \
        if (destructor) { \
            void *ptr = vec + idx * vector_base_to_header(vec)->item_size; \
            destructor(ptr); \
        } \
    } while(0);

#define vector_empty(vec) \
    ( vector_size((vec)) == 0 )

#define vector_push_back(vec, val) \
    do { \
        vector_create_header(vec); \
        if (vector_size(vec) >= vector_capacity(vec)) { \
            size_t vec_cap = vector_capacity(vec); \
            size_t new_cap = ((vec_cap) ? vec_cap * 2 : 1); \
            vector_grow(vec, new_cap); \
        } \
        (vec)[vector_size(vec)] = (val); \
        __vector_set_size(vec, vector_size(vec) + 1); \
    } while(0);

#define vector_push_back_many(vec, type, ...) \
    do { \
        type arr[] = { __VA_ARGS__ }; \
        for (size_t i = 0; i < sizeof(arr)/sizeof(arr[0]); i++) { \
            vector_push_back(vec, arr[i]); \
        } \
    } while(0);

#define vector_find_str_by(vec, find_by, what, result) \
    do {\
        result = NULL; \
        for (size_t i = 0; i < vector_size(vec); i++) { \
            if (strcmp(what, vec[i]find_by) == 0) { \
                result = &vec[i]; \
            } \
        } \
    } while(0);

#define vector_set_destructor(vec, d_tor) \
    do { \
        vector_create_header(vec); \
        ((VecHeader *)vector_base_to_header(vec))->destructor = (d_tor); \
    } while(0);

#define vector_erase(vec, idx) \
    do { \
        void *_vector = vec; \
        if (vector_size(vec) <= idx) { \
            break; \
        } \
        __vector_free_item(_vector, idx); \
        if (vector_size(vec) - 1 == idx) { \
            __vector_set_size(vec, vector_size(vec) - 1); \
            break; \
        } \
        memcpy((void *)&vec[idx], (void *)&vec[idx + 1], \
                (vector_size(vec) - idx) * sizeof(*(vec))); \
        __vector_set_size(vec, vector_size(vec) - 1); \
    } while(0);

#define vector_clean(vector) \
    do { \
        void *vec = vector; \
        if (vec == NULL) { \
            break; \
        } \
        for (size_t __i = 0; __i < vector_size(vec); __i++) { \
            __vector_free_item(vec, __i); \
        } \
        memset(vec, 0, vector_size(vec)); \
        __vector_set_size(vec, 0); \
    } while(0);

#define vector_reserve(vec, new_cap) \
    do { \
        vector_create_header(vec); \
        if (vector_capacity(vec) < new_cap) { \
            vector_grow(vec, new_cap); \
        } \
        __vector_set_capacity(vec, new_cap); \
    } while(0);

#define foreach(item_type, item, vec) \
    for (item_type *item = vec; item < vec + vector_size(vec); item++)\

#define vector_init(vec, type, destructor, ...) \
    do { \
        vector_set_destructor(vec, destructor); \
        vector_push_back_many(vec, type, __VA_ARGS__); \
    } while(0);

void free_vector(void *vector);

#ifdef VECTOR_IMPLEMENTATION
void free_vector(void *vector) {
        void *vec = *(void **)vector;
        if (vec == NULL) {
            return;
        }
        for (size_t __i = 0; __i < vector_size(vec); __i++) {
            __vector_free_item(vec, __i);
        }
        free(vector_base_to_header(vec));
}
#endif
#endif
