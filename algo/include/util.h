#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>


typedef struct number_metadata {
        int number;
        int digit_count;
        int digit_sum;
} NumberMetadata;


void die(int status, const char *errfmt, ...);
int eprintf(const char *format, ...);

int input_int();
size_t input_size_t();
char *inputline(FILE *);

void print_int_array(int *arr, size_t length);

NumberMetadata get_number_metadata(int number);

void *xmalloc(size_t size);
void *xcalloc(size_t nmemb, size_t size);
void *xrealloc(void *ptr, size_t size);

char * strdup(const char *s);

size_t strcount(const char *s, char c);
size_t strncount(const char *s, char c, size_t n);

int popcount(uint64_t x);
int popcount8(uint8_t x);
