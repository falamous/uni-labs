#include "struct.h"


/*
 * A stack is just a thread safe vector without the structure garantee.
 * Nothing can fail except for 0 size vectors.
 */

#ifndef LINKED_LIST_STACK

void stack_init(Stack *stack) {
        mutex_init(&stack->mutex, NULL);
        vector_init(&stack->stack, 0);
        vector_reserve(&stack->stack, 1024);
}

void stack_push(Stack *stack, StructValue val) {
        mutex_lock(&stack->mutex);
        vector_pb(&stack->stack, val);
        mutex_unlock(&stack->mutex);
}

int stack_pop(Stack *stack, StructValue *val) {
        int res;

        mutex_lock(&stack->mutex);
        res = vector_pop(&stack->stack, val);
        mutex_unlock(&stack->mutex);

        return res;
}

int stack_top(Stack *stack, StructValue *val) {
        int res;

        mutex_lock(&stack->mutex);
        res = vector_last(&stack->stack, val);
        mutex_unlock(&stack->mutex);

        return res;
}

void stack_destroy(Stack *stack) {
        vector_destroy(&stack->stack);
        mutex_destroy(&stack->mutex);
}

#endif
