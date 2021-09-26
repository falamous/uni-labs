#include "struct.h"


/*
 * A stack is just a thread safe linked list with no gurantees.
 * Nothing can fail except for 0 size vectors.
 */

#ifdef DLINKED_LIST_STACK

void stack_init(Stack *stack) {
        mutex_init(&stack->mutex, NULL);
        stack->stack = NULL;
}

void stack_push(Stack *stack, StructValue val) {
        struct StackNode *node;

        mutex_lock(&stack->mutex);
        node = xmalloc(sizeof(struct StackNode));

        node->val = val;
        node->bp = stack->stack;

        stack->stack = node;
        mutex_unlock(&stack->mutex);
}

int stack_pop(Stack *stack, StructValue *val) {
        int res;
        struct StackNode *bp;

        mutex_lock(&stack->mutex);
        if (stack->stack == NULL) {
                res = 1;
        } else {
                res = 0;
                *val = stack->stack->val;
                bp = stack->stack;
                stack->stack = bp->bp;
                free(bp);
        }
        mutex_unlock(&stack->mutex);

        return res;
}

int stack_top(Stack *stack, StructValue *val) {
        int res;

        mutex_lock(&stack->mutex);
        if (stack->stack == NULL) {
                res = 1;
        } else {
                res = 0;
                *val = stack->stack->val;
        }
        mutex_unlock(&stack->mutex);

        return res;
}

void stack_destroy(Stack *stack) {
        struct StackNode *bp;
        mutex_destroy(&stack->mutex);

        while (stack->stack != NULL) {
                bp = stack->stack;
                stack->stack = bp->bp;
                free(bp);
        }
}

#endif
