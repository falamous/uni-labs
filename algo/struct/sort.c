#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <alloca.h>

void merge_sorted(void *a, size_t a_nmemb,
                void *b, size_t b_nmemb,
                void *res,
                size_t size, int (*compar)(const void *, const void *)) {
        size_t ap = 0;
        size_t bp = 0;
        size_t i = 0;

        while(ap < a_nmemb || bp < b_nmemb){
                if (ap >= a_nmemb) {
                        /* res[i++] = b[bp++]; */
                        memcpy(res + (i++) * size, b + (bp++) * size, size);
                        continue;
                }
                if (bp >= b_nmemb) {
                        /* res[i++] = a[ap++]; */
                        memcpy(res + (i++) * size, a + (ap++) * size, size);
                        continue;
                }
                if (compar((void *)(a + ap * size), (void *)(b + bp * size)) <= 0) {
                        /* res[i++] = a[ap++]; */
                        memcpy(res + (i++) * size, a + (ap++) * size, size);
                } else {
                        /* res[i++] = b[bp++]; */
                        memcpy(res + (i++) * size, b + (bp++) * size, size);
                }
        }
}

void mergesort(void *base, size_t nmemb, size_t size,
                int (*compar)(const void *, const void *)) {
        const size_t base_size = size * nmemb;
        void *to_merge = base;
        void *merged;
        void *tmp;
        bool merged_malloced;

        size_t bucket_size;
        size_t i;

        if (size * nmemb > 4096){
                merged_malloced = true;
                merged = malloc(base_size);
        } else {
                merged_malloced = false;
                merged = alloca(base_size);

        }

        for (bucket_size = 1; bucket_size < nmemb; bucket_size *= 2) {
                for (i = 0; i < nmemb; i += bucket_size * 2) {
                        if (i + bucket_size * 2 < nmemb) {
                                merge_sorted(
                                                to_merge + i * size, bucket_size,
                                                to_merge + (i + bucket_size) * size, bucket_size,
                                                merged + i * size,
                                                size, compar);
                        } else if (i + bucket_size < nmemb) {
                                merge_sorted(
                                                to_merge + i * size, bucket_size,
                                                to_merge + (i + bucket_size) * size, nmemb - i - bucket_size,
                                                merged + i * size,
                                                size, compar);

                        }
                }
                /* swap(merged, to_merge) */
                tmp = merged;
                merged = to_merge;
                to_merge = tmp;
        }

        if (to_merge != base) {
                /* because of the swap the final array ends up in to_merge */
                memcpy(base, to_merge, base_size);
        }
        if (merged_malloced) {
                free(merged);
        }
}
