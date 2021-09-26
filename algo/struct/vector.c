#include <stdlib.h>
#include <string.h>
#include "struct.h"
#include "util.h"


/*
 * Straight-forward vector implementation
 * Nothing throws erros except pop (back evidently) and last.
 */

void vector_init(Vector *vec, size_t len) {
        vec->len = len;
        vec->cap = len;
        vec->arr = malloc(len * sizeof(Val));
}

void vector_from_arr(Vector *vec, Val *arr, size_t len) {
        vector_init(vec, len);
        memcpy(vec->arr, arr, len * sizeof(Val));
}

void vector_resize(Vector *vec, size_t len) {
        if (len > vec->cap) {
                vector_reserve(vec, len);
        }
        vec->len = len;
}

void vector_reserve(Vector *vec, size_t cap) {
        if (vec->len > cap) {
            return;
        }
        vec->arr = xrealloc(vec->arr, cap * sizeof(*vec->arr));
        vec->cap = cap;
}

void vector_pb(Vector *vec, Val val) {
        while (vec->len == vec->cap) {
                vector_reserve(vec, (vec->cap + 1) * 2);
        }
        vec->arr[vec->len++] = val;
}

int vector_pop(Vector *vec, Val *val) {
        if (vec->len == 0) {
                return 1;
        }

        if (val) { *val = vec->arr[vec->len]; }
        vec->len--;
        return 0;
}

int vector_last(Vector *vec, Val *val) {
        if (vec->len == 0) {
                return 1;
        }

        if (val) { *val = vec->arr[vec->len - 1]; }
        return 0;
}

void vector_destroy(Vector *vec) {
        free(vec->arr);
}
