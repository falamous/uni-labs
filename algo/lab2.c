#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include "struct.h"
#include "util.h"

int istoken(char c) {
        return isalnum(c) || c == '_' || c == '.';
}

int isop(char c) {
        return c == '+' || c == '-' || c == '*' || c == '/';
}

char *allocchr(char c) {
        char *res = xmalloc(2);
        res[0] = c;
        res[1] = '\0';
        return res;
}

int opprec(char op) {
        switch (op) {
                case '+' :
                case '-' :
                        return 0;
                case '*' :
                case '/' :
                        return 1;
                default :
                        return -1;
        }
}


int preccmp(char op1, char op2) {
        /* assume operator precedence is far from max values */
        return opprec(op1) - opprec(op2);
}


Vector shunting_yard(FILE *stream) {
        Vector tmp_token;
        size_t i;
        Val op;
        int c;

        Stack operator_stack;

        Vector output;
        char *output_token;

        stack_init(&operator_stack);
        vector_init(&tmp_token, 0);
        vector_init(&output, 0);

        vector_reserve(&tmp_token, 1024);
        vector_reserve(&output, 1024);

        while ((c = fgetc(stream)) != EOF) {
                if (isspace(c)) {
                        continue;
                } else if (istoken(c)) {
                        vector_resize(&tmp_token, 0);
                        do {
                                vector_pb(&tmp_token, (Val){.i = c});
                        } while((c = fgetc(stream)) != EOF && istoken(c));
                        ungetc(c, stream);

                        output_token = malloc(tmp_token.len + 1);
                        for (i = 0; i < tmp_token.len; i++) {
                                output_token[i] = (char)tmp_token.arr[i].i;
                        }
                        output_token[tmp_token.len] = '\0';

                        vector_pb(&output, (Val){.p = output_token});
                } else if (isop(c)) {
                        while (stack_top(&operator_stack, &op) == 0 && op.i != '(' && preccmp(op.i, c) >= 0) {
                                stack_pop(&operator_stack, &op);
                                vector_pb(&output, (Val)(void *)allocchr(op.i));
                        }
                        stack_push(&operator_stack, (Val){.i = c});
                } else if (c == '(') {
                        stack_push(&operator_stack, (Val){.i = c});
                } else if (c == ')') {
                        while (stack_top(&operator_stack, &op) == 0 && op.i != '(') {
                                stack_pop(&operator_stack, &op);
                                vector_pb(&output, (Val)(void *)allocchr(op.i));
                        }
                        if (stack_pop(&operator_stack, &op) != 0) {
                                die(2, "Mismatched parenthesis ')'.\n");
                        }
                } else {
                        die(2, "Unexpected character '\\x%02hhx'.", c);
                }
        }
        while (stack_pop(&operator_stack, &op) == 0) {
                if (op.i == '(') {
                        die(2, "Mismatched parenthesis '('.\n");
                }
                vector_pb(&output, (Val){.p = allocchr(op.i)});

        }
        stack_destroy(&operator_stack);
        vector_destroy(&tmp_token);

        return output;
}

int main() {
        Vector postfix_formula;
        size_t i;
        postfix_formula = shunting_yard(stdin);

        for (i = 0; i != postfix_formula.len - 1; i++) {
                fwrite(postfix_formula.arr[i].p, 1, strlen(postfix_formula.arr[i].p), stdout);
                putchar(' ');

        }

        if (postfix_formula.len != 0) {
                puts(postfix_formula.arr[i].p);
        }
        vector_destroy(&postfix_formula);
}
