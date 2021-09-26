#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <alloca.h>

void merge_sorted(void *a, size_t a_nmemb,
                void *b, size_t b_nmemb,
                void *res,
                size_t size, int (*compar)(const void *, const void *));
void mergesort(void *base, size_t nmemb, size_t size,
                int (*compar)(const void *, const void *));
