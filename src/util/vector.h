#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>

#define VEC_PUSH_LITERAL(vec,typ,expr) {\
    typ z = (expr);\
    vec_push(vec, &z);\
}

/*
Never, never save pointers when there are elements
inserted into the buffer! It can be realloc'ed!
*/

typedef struct
{
    size_t elem_size;
    size_t elem_count;
    size_t buffer_size;
    void *buffer;
} Vector;

bool vec_ensure_size(Vector *vec, size_t needed_size);
Vector vec_create(size_t elem_size, size_t start_size);
void vec_reset(Vector *vec);
void vec_destroy(Vector *vec);
void *vec_get(Vector *vec, size_t index);
bool vec_push(Vector *vec, void *elem);
bool vec_push_many(Vector *vec, size_t num, void *elem);
void *vec_push_empty(Vector *vec);
void *vec_pop(Vector *vec);
void *vec_peek(Vector *vec);
size_t vec_count(Vector *vec);