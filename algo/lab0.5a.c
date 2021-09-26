#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "util.h"
#include "sort.h"

/* char *filter_line(const char *line) { */
/*         const int first_popcount = popcount8(line[0]); */
/*         char *filtered; */
/*         char *fp; */
/*         size_t filtered_len = 0; */

/*         while (line[filtered_len] != '\0') { */
/*                 if (popcount8(*line) == first_popcount) { */
/*                         ++filtered_len; */
/*                 } */
/*         } */

/*         filtered = xmalloc(filtered_len); */
/*         fp = filtered; */
/*         for (; *line != '\0'; ++line) { */
/*                 if (popcount8(*line) == first_popcount) { */
/*                         *(fp++) = *(line); */
/*                 } */
/*         } */

/*         *fp = '\0'; */
/*         return filtered; */
/* } */

size_t strword(char **s) {
        size_t wordlen;

        while (**s != '\0' && isspace(**s)) {
                ++(*s);
        }

        wordlen = 0;
        while ((*s)[wordlen] != '\0' && !isspace((*s)[wordlen])) {
                ++wordlen;
        }
        return wordlen;

}

char *filter_line(char *line) {
        char *lineptr;
        char *newlineptr;

        size_t one_count;
        size_t wordlen;

        newlineptr = xmalloc(strlen(line) + 1);
        newlineptr[0] = '\0';
        lineptr = line;

        wordlen = strword(&lineptr);
        one_count = strncount(lineptr, '1', wordlen);
        while (*lineptr != '\0') {
                wordlen = strword(&lineptr);
                if (strncount(lineptr, '1', wordlen) == one_count) {
                        strncat(newlineptr, lineptr, wordlen);
                        strcat(newlineptr, " ");
                }
                lineptr += wordlen;
        }

        return newlineptr;
}

int main() {
        char *line;
        char *filtered_line;

        for(;;) {
                line = inputline(stdin);
                if (line == NULL) {
                        return 0;
                }
                filtered_line = filter_line(line);
                puts(filtered_line);

                free(line);
                free(filtered_line);
        }

        return 0;
}
