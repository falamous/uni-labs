#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include "util.h"

/* #define INPUTLINE_REALLOC_TO_EXACT_SIZE */

void die(int status, const char *errfmt, ...) {
	va_list ap;

	va_start(ap, errfmt);
	vfprintf(stderr, errfmt, ap);
	va_end(ap);
	exit(status);
}

int eprintf(const char *format, ...){
        int res;
	va_list ap;

	va_start(ap, format);
	res = vfprintf(stderr, format, ap);
	va_end(ap);

        return res;
}


void *xmalloc(size_t size) {
        void *ptr;

        ptr = malloc(size);
        if (ptr == NULL) {
                die(1, "Could not malloc(%zu).", size);
        }

        return ptr;
}

void *xcalloc(size_t nmemb, size_t size) {
        void *ptr;

        ptr = calloc(nmemb, size);
        if (ptr == NULL) {
                die(1, "Could not calloc(%zu, %zu).", nmemb, size);
        }

        return ptr;
}

void *xrealloc(void *ptr, size_t size) {
        void *nptr;

        nptr = realloc(ptr, size);
        if (nptr == NULL && size != 0) {
                die(1, "Could not realloc(%p, %zu).", ptr, size);
        }

        return nptr;
}


int input_int() {
        /* reads int from sdtin asking to enter it nicely */
        /* dies with exit(EOF) on EOF */

        int res;
        int read;
        while ((read = scanf("%i", &res)) != 1){
                if (read == 0) {
                        eprintf("Could not read int enter again> ");
                        scanf("%*[^0-9]");
                }
                if (read < 0){
                        die(EOF, "Got an unexpected EOF.");
                }
        }

        return res;
}

size_t input_size_t() {
        /* reads size_t from sdtin asking to enter it nicely */
        /* dies with exit(EOF) on EOF */

        size_t res;
        int read;
        while ((read = scanf("%zu", &res)) != 1){
                if (read == 0) {
                        eprintf("Could not read size_t enter again> ");
                        scanf("%*[^0-9]");
                }
                if (read < 0){
                        die(EOF, "Got an unexpected EOF.");
                }
        }

        return res;
}

char *inputline(FILE *stream){
        size_t buf_size;
        char *buf;
        char format[32];
        size_t buf_len;
        size_t read;

        buf_len = 0;
        buf_size = 128;
        buf = xmalloc(buf_size);
        buf[0] = '\0';

        do {
                if (buf_size == buf_len + 1){
                        buf_size *= 2;
                        buf = xrealloc(buf, buf_size);
                }
                snprintf(format, sizeof(format), "%%%zu[^\n]", buf_size - buf_len - 1, &read);
                if (fscanf(stream, format, buf) == EOF) {
                        if (buf_len == 0) {
                                free(buf);
                                return NULL;
                        }
                }
                read = strlen(buf + buf_len);
                buf_len += read;
        } while (read != 0);
        fscanf(stream, "%*1[\n]");
#ifdef INPUTLINE_REALLOC_TO_EXACT_SIZE
        buf = xrealloc(buf, buf_len);
#endif

        return buf;
}


void print_int_array(int *arr, size_t len) {
        size_t i;

        if (len == 0){
                puts("");
                return;
        }

        for (i = 0; i < len - 1; ++i){
                printf("%i ", arr[i]);
        }
        printf("%i\n", arr[i]);
}

NumberMetadata get_number_metadata(int number) {
        /* gets the digit count ant sum of the number */
        /* cannot fail */
        NumberMetadata meta;
        meta.digit_count = 0;
        meta.digit_sum = 0;

        meta.number = number;
        if (number < 0) {
                number = -number;
        }
        if (number == 0) {
                /* zero is a special case */
                meta.digit_count = 1;
        }

        while (number != 0) {
                ++meta.digit_count;
                if (number > 0) {
                        meta.digit_sum += number % 10;
                        number /= 10;
                } else {
                        /* rounding down is not what we want for negative numbers */
                        meta.digit_sum += 10 - number % 10;
                        number = (number + 9) / 10;
                }
        }
        return meta;
}

char * strdup(const char *s) {
    size_t len;
    char *new_string;

    len = strlen(s);

    new_string = xmalloc(len + 1);
    memcpy(new_string, s, len);
    new_string[len] = '\0';
    return new_string;
}

size_t strcount(const char *s, char c) {
        size_t count = 0;
        for (; *s; ++s) {
                if (c == *s) {
                        ++count;
                }
        }
        return count;
}

size_t strncount(const char *s, char c, size_t n) {
        size_t count = 0;
        for (; *s && n--; ++s) {
                if (c == *s) {
                        ++count;
                }
        }
        return count;
}

int popcount(uint64_t x){
        x -= (x >> 1) & 0x5555555555555555;
        x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);
        x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0f;
        return (x * 0x0101010101010101) >> 56;
}

int popcount8(uint8_t x){
        int count = 0;
        for(int i = 0; i < 8; i++){
                if (x & (1 << i)) {
                        count += 1;
                }
        }
        return count;
}
