/* 
 * Вариант 35
 * Дана последовательность из n натуральных чисел.
 * Для каждого числа исходной последовательности подсчитать количество и сумму цифр в записи числа.
 * Сформировать две новых последовательности, состоящие соответственно из количества и суммы цифр в записи чисел, стоящих на нечетных местах в исходной последовательности. Упорядочить полученные последовательности по возрастанию. Вывести исходную и полученные последовательности на экран.
 * В программе использовать функцию, которая определяет количество и сумму цифр в записи числа. 
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "util.h"
#include "sort.h"


typedef struct LinkedString {
        char c;
        struct LinkedString *np;
} LinkedString;


LinkedString *input_linkedline() {
        LinkedString *ls;
        LinkedString **lsp;
        char *s;
        size_t i;
        size_t slen;

        s = inputline(stdin);
        if (s == NULL || (slen = strlen(s)) == 0) {
                free(s);
                return NULL;
        }

        ls = malloc(sizeof(LinkedString));
        lsp = &ls;
        for (i = 0; i < slen; i++) {
                *lsp = malloc(sizeof(LinkedString));
                (*lsp)->c = s[i];
                (*lsp)->np = NULL;
                lsp = &(*lsp)->np;
        }

        free(s);
        return ls;
}

LinkedString *linkedword (LinkedString **line) {
        LinkedString **wp;
        LinkedString *word;

        word = NULL;
        wp = &word;
        while (*line != NULL && isspace((*line)->c)) {
                *line = (*line)->np;
        }

        while (*line != NULL && !isspace((*line)->c)) {
                *wp = malloc(sizeof(LinkedString));
                (*wp)->c = (*line)->c;
                (*wp)->np = NULL;
                wp = &(*wp)->np;
                *line = (*line)->np;
        }
        return word;
}

size_t linkedlen(LinkedString *s) {
        size_t len;

        while (s != NULL) {
                s = s->np;
                len++;
        }

        return len;
}

void linkedadvance(LinkedString ***s) {
        while ((**s) != NULL) {
                *s = &(**s)->np;
        }
}

void linkedcat(LinkedString **dest, LinkedString *src) {
        linkedadvance(&dest);
        *dest = src;
        linkedadvance(&dest);
}

void linkedfree(LinkedString *ptr) {
        LinkedString *nptr;

        while (ptr != NULL) {
                nptr = ptr->np;
                free(ptr);
                ptr = nptr;
        }
}

void linkedpb(LinkedString **s, char c) {
        linkedadvance(&s);

        *s = malloc(sizeof(LinkedString));
        (*s)->c = c;
        (*s)->np = NULL;
}

LinkedString *filter_linkedline(LinkedString *line) {
        LinkedString *new_line;
        LinkedString *new_word;

        LinkedString *line_end;
        LinkedString *new_line_end;
        size_t first_len;

        line_end = line;
        new_line = linkedword(&line_end);
        first_len = linkedlen(new_line);
        if (new_line == NULL) {
                return NULL;
        }

        new_line_end = new_line;

        while (line_end != NULL) {
                new_word = linkedword(&line_end);

                if (linkedlen(new_word) == first_len && first_len != 0) {
                        linkedpb(&new_line_end, ' ');
                        linkedcat(&new_line_end, new_word);
                } else {
                        linkedfree(new_word);
                }

        }


        return new_line;
}

int linkedputs(LinkedString *s) {
        while (s != NULL) {
                putchar(s->c);
                s = (*s)->np;
        }
        return putchar('\n');
}

int main() {
        LinkedString *line;
        LinkedString *filtered_line;

        for (;;) {
                line = input_linkedline();
                if (line == NULL) {
                        break;
                }
                filtered_line = filter_linkedline(line);

                linkedputs(&filtered_line);

                linkedfree(line);
                linkedfree(filtered_line);
        }
}
