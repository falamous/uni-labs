#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdlib.h>

typedef double m_t;

typedef struct int_matrix {
        m_t **matrix;
        size_t row_start; /* for slices */
        size_t height;
        size_t width;
} IntMatrix;

void intmatrix_init(IntMatrix *matrix, size_t height, size_t width, m_t element) {
    size_t i;
    size_t j;
    matrix->height = height;
    matrix->width = width;
    matrix->row_start = 0;

    matrix->matrix = malloc(sizeof(int *) * height);
    for (i = 0; i < height; i++) {
        matrix->matrix[i] = malloc(sizeof(m_t) * width);
        for (j = 0; j < width; j++) {
            matrix->matrix[i][j] = element;
        }
    }
}

IntMatrix *intmatrix_new(size_t height, size_t width, m_t element) {
    IntMatrix *matrix;
    matrix = malloc(sizeof(*matrix));
    intmatrix_init(matrix, height, width, element);
    return matrix;
}

void intmatrix_destroy(IntMatrix *matrix) {
    size_t i;
    for (i = 0; i < matrix->height; i++) {
        free(matrix->matrix[i]);
    }
    free(matrix->matrix);
}

void intmatrix_free(IntMatrix *matrix) {
    intmatrix_destroy(matrix);
    free(matrix);
}

IntMatrix *intmatrix_slice(IntMatrix *matrix, size_t i, size_t j, size_t height, size_t width) {
    IntMatrix *slice;

    slice = malloc(sizeof(*slice));
    slice->width = width;
    slice->height = height;
    slice->matrix = matrix->matrix + i;
    slice->row_start = matrix->row_start + j;
    return slice;
}

void intmatrix_add(IntMatrix *a, IntMatrix *b) {
    size_t i;
    size_t j;
    size_t height;
    size_t width;

    height = a->height;
    if (b->height < height) {
        height = b->height;
    }
    width = a->width;
    if (b->width < width) {
        width = b->width;
    }

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            a->matrix[i][j + a->row_start] += b->matrix[i][j + b->row_start];
        }
    }
}

void intmatrix_sub(IntMatrix *a, IntMatrix *b) {
    size_t i;
    size_t j;
    size_t height;
    size_t width;

    height = a->height;
    if (b->height < height) {
        height = b->height;
    }
    width = a->width;
    if (b->width < width) {
        width = b->width;
    }

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            a->matrix[i][j + a->row_start] -= b->matrix[i][j + b->row_start];
        }
    }
}

void intmatrix_set(IntMatrix *a, IntMatrix *b) {
    size_t i;
    size_t j;
    size_t height;
    size_t width;

    height = a->height;
    if (b->height < height) {
        height = b->height;
    }
    width = a->width;
    if (b->width < width) {
        width = b->width;
    }

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            a->matrix[i][j + a->row_start] = b->matrix[i][j + b->row_start];
        }
    }
}

IntMatrix *intmatrix_transpose_into(IntMatrix *a, IntMatrix *b) {
    for (int i = 0; i < a->height; ++i) {
        for (int j = 0; j < a->width; ++j) {
            b->matrix[j][i/* + b->row_start */] = a->matrix[i][j + a->row_start];
        }
    }
    return b;
}

IntMatrix *intmatrix_transpose(IntMatrix *a) {
    IntMatrix *b;
    b = intmatrix_new(a->width, a->height, 0);
    intmatrix_transpose_into(a, b);
    return b;
}

IntMatrix *intmatrix_mult_transposed(IntMatrix *a, IntMatrix *b) {
    IntMatrix *c;
    c = intmatrix_new(a->height, b->width, 0);

    for (int i = 0; i < a->height; ++i) {
        for (int j = 0; j < a->width; ++j) {
            c->matrix[i][j] = 0;
            for (int k = 0; k < a->width; ++k) {
                /* c->matrix[i][j] += a->matrix[i][k + a->row_start] * b->matrix[k][j + b->row_start]; */
                c->matrix[i][j] += a->matrix[i][k + a->row_start] * b->matrix[j][k + b->row_start];
            }
        }
    }
    return c;
}

IntMatrix *intmatrix_mult_transposed_unrolled_16(IntMatrix *a, IntMatrix *b) {
    IntMatrix *c;
    c = intmatrix_new(a->height, b->width, 0);

    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            c->matrix[i][j] = 0;
            for (int k = 0; k < 16; ++k) {
                /* c->matrix[i][j] += a->matrix[i][k + a->row_start] * b->matrix[k][j + b->row_start]; */
                c->matrix[i][j] += a->matrix[i][k + a->row_start] * b->matrix[j][k + b->row_start];
            }
        }
    }
    return c;
}


IntMatrix *intmatrix_mult(IntMatrix *a, IntMatrix *b) {
    IntMatrix *c;
    b = intmatrix_transpose(b);
    c = intmatrix_mult_transposed(a, b);
    intmatrix_free(b);


    return c;
}

IntMatrix *strassen_proper(IntMatrix *a, IntMatrix *b) {
    IntMatrix *c;
    IntMatrix *M[7];

    IntMatrix *a_slice[2][2];
    IntMatrix *b_slice[2][2];
    IntMatrix *c_slice[2][2];
    IntMatrix *mult1;
    IntMatrix *mult2;
    size_t n;


    n = a->height/2;

    if (n < 32) {
        return intmatrix_mult_transposed(a, b);
    }
    /* switch (a->height) { */
    /*     case 16: */
    /*         return intmatrix_mult_transposed_unrolled_16(a, b); */
    /*     case 8: */
    /*     case 4: */
    /*     case 2: */
    /*     case 1: */
    /*         return intmatrix_mult_transposed(a, b); */
    /*     default: */
    /*         break; */
    /* } */


    c = intmatrix_new(2 * n, 2 * n, 0);

    a_slice[0][0] = intmatrix_slice(a, 0, 0, n, n);
    a_slice[0][1] = intmatrix_slice(a, 0, n, n, n);
    a_slice[1][0] = intmatrix_slice(a, n, 0, n, n);
    a_slice[1][1] = intmatrix_slice(a, n, n, n, n);

    /* b_slice[0][0] = intmatrix_slice(b, 0, 0, n, n); */
    /* b_slice[0][1] = intmatrix_slice(b, 0, n, n, n); */
    /* b_slice[1][0] = intmatrix_slice(b, n, 0, n, n); */
    /* b_slice[1][1] = intmatrix_slice(b, n, n, n, n); */
    b_slice[0][0] = intmatrix_slice(b, 0, 0, n, n);
    b_slice[0][1] = intmatrix_slice(b, n, 0, n, n);
    b_slice[1][0] = intmatrix_slice(b, 0, n, n, n);
    b_slice[1][1] = intmatrix_slice(b, n, n, n, n);

    c_slice[0][0] = intmatrix_slice(c, 0, 0, n, n);
    c_slice[0][1] = intmatrix_slice(c, 0, n, n, n);
    c_slice[1][0] = intmatrix_slice(c, n, 0, n, n);
    c_slice[1][1] = intmatrix_slice(c, n, n, n, n);

    /* c_slice[0][0] = intmatrix_slice(c, 0, 0, n, n); */
    /* c_slice[0][1] = intmatrix_slice(c, n, 0, n, n); */
    /* c_slice[1][0] = intmatrix_slice(c, 0, n, n, n); */
    /* c_slice[1][1] = intmatrix_slice(c, n, n, n, n); */

    mult1 = intmatrix_new(n, n, 0);
    mult2 = intmatrix_new(n, n, 0);

    intmatrix_set(mult1, a_slice[0][0]);
    intmatrix_add(mult1, a_slice[1][1]);
    intmatrix_set(mult2, b_slice[0][0]);
    intmatrix_add(mult2, b_slice[1][1]);
    M[0] = strassen_proper(mult1, mult2);
    intmatrix_set(mult1, a_slice[1][0]);
    intmatrix_add(mult1, a_slice[1][1]);
    intmatrix_set(mult2, b_slice[0][0]);
    M[1] = strassen_proper(mult1, mult2);
    intmatrix_set(mult1, a_slice[0][0]);
    intmatrix_set(mult2, b_slice[0][1]);
    intmatrix_sub(mult2, b_slice[1][1]);
    M[2] = strassen_proper(mult1, mult2);
    intmatrix_set(mult1, a_slice[1][1]);
    intmatrix_set(mult2, b_slice[1][0]);
    intmatrix_sub(mult2, b_slice[0][0]);
    M[3] = strassen_proper(mult1, mult2);
    intmatrix_set(mult1, a_slice[0][0]);
    intmatrix_add(mult1, a_slice[0][1]);
    intmatrix_set(mult2, b_slice[1][1]);
    M[4] = strassen_proper(mult1, mult2);
    intmatrix_set(mult1, a_slice[1][0]);
    intmatrix_sub(mult1, a_slice[0][0]);
    intmatrix_set(mult2, b_slice[0][0]);
    intmatrix_add(mult2, b_slice[0][1]);
    M[5] = strassen_proper(mult1, mult2);
    intmatrix_set(mult1, a_slice[0][1]);
    intmatrix_sub(mult1, a_slice[1][1]);
    intmatrix_set(mult2, b_slice[1][0]);
    intmatrix_add(mult2, b_slice[1][1]);
    M[6] = strassen_proper(mult1, mult2);

    intmatrix_set(c_slice[0][0], M[0]);
    intmatrix_add(c_slice[0][0], M[3]);
    intmatrix_sub(c_slice[0][0], M[4]);
    intmatrix_add(c_slice[0][0], M[6]);

    intmatrix_set(c_slice[0][1], M[2]);
    intmatrix_add(c_slice[0][1], M[4]);

    intmatrix_set(c_slice[1][0], M[1]);
    intmatrix_add(c_slice[1][0], M[3]);

    intmatrix_set(c_slice[1][1], M[0]);
    intmatrix_sub(c_slice[1][1], M[1]);
    intmatrix_add(c_slice[1][1], M[2]);
    intmatrix_add(c_slice[1][1], M[5]);


    /* intmatrix_set(c_slice[1][1], M[0]); */
    /* intmatrix_add(c_slice[1][1], M[3]); */
    /* intmatrix_sub(c_slice[1][1], M[4]); */
    /* intmatrix_add(c_slice[1][1], M[6]); */

    /* intmatrix_set(c_slice[1][0], M[2]); */
    /* intmatrix_add(c_slice[1][0], M[4]); */

    /* intmatrix_set(c_slice[0][1], M[1]); */
    /* intmatrix_add(c_slice[0][1], M[3]); */

    /* intmatrix_set(c_slice[0][0], M[0]); */
    /* intmatrix_sub(c_slice[0][0], M[1]); */
    /* intmatrix_add(c_slice[0][0], M[2]); */
    /* intmatrix_add(c_slice[0][0], M[5]); */

    intmatrix_free(M[0]);
    intmatrix_free(M[1]);
    intmatrix_free(M[2]);
    intmatrix_free(M[3]);
    intmatrix_free(M[4]);
    intmatrix_free(M[5]);
    intmatrix_free(M[6]);

    intmatrix_free(mult1);
    intmatrix_free(mult2);

    free(a_slice[0][0]);
    free(a_slice[0][1]);
    free(a_slice[1][0]);
    free(a_slice[1][1]);

    free(b_slice[0][0]);
    free(b_slice[0][1]);
    free(b_slice[1][0]);
    free(b_slice[1][1]);

    free(c_slice[0][0]);
    free(c_slice[0][1]);
    free(c_slice[1][0]);
    free(c_slice[1][1]);
    return c;
}

IntMatrix *strassen(IntMatrix *a, IntMatrix *b) {
    IntMatrix *c;
    IntMatrix *proper_a;
    IntMatrix *proper_b;
    IntMatrix *proper_c;
    size_t n;
    size_t i;
    size_t j;

    if (a->width != b->height) {
        return NULL;
    }

    n = 1;
    while (n < a->height) {
        n *= 2;
    }
    while (n < a->width) {
        n *= 2;
    }
    while (n < b->height) {
        n *= 2;
    }
    while (n < b->width) {
        n *= 2;
    }

    proper_a = intmatrix_new(n, n, 0);
    proper_b = intmatrix_new(n, n, 0);
    intmatrix_set(proper_a, a);
    intmatrix_transpose_into(b, proper_b);

    proper_c = strassen_proper(proper_a, proper_b);
    c = intmatrix_new(a->height, b->width, 0);
    intmatrix_set(c, proper_c);

    intmatrix_free(proper_a);
    intmatrix_free(proper_b);
    intmatrix_free(proper_c);
    return c;
}

double mtime() {
    struct timeval ctime;
    gettimeofday(&ctime, NULL);
    return ctime.tv_usec/1e6 + ctime.tv_sec;
}

int main() {
    IntMatrix *a;
    IntMatrix *b;
    IntMatrix *c;
    IntMatrix *d;
    double t;
    size_t n = 1000;

    a = intmatrix_new(n, n, 0);
    b = intmatrix_new(n, n, 0);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            a->matrix[i][j] = rand() % 1337;
            b->matrix[i][j] = rand() % 1337;
        }
    }
   /* a->matrix[0][1] = 0; */
   /* a->matrix[1][0] = 0; */
   /*  a->matrix[1][1] = 0; */
   /*  a->matrix[0][0] = 1; */

    t = mtime();
    c = strassen(a, b);
    printf("%lf\n", mtime() - t);

    t = mtime();
    d = intmatrix_mult(a, b);
    printf("%lf\n", mtime() - t);
    /* for (int i = 0; i < n; i++) { */
    /*     for (int j = 0; j < n; j++) { */
    /*             printf("%i ", d->matrix[i][j]); */
    /*     } */
    /*     puts(""); */
    /* } */

    /* for (int i = 0; i < n; i++) { */
    /*     for (int j = 0; j < n; j++) { */
    /*             printf("%i ", c->matrix[i][j]); */
    /*     } */
    /*     puts(""); */
    /* } */

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (d->matrix[i][j] != c->matrix[i][j]) {
                printf("%i %i\n", i, j);
            }
        }
    }
}
