/*
 * Вариант 29
 * Из входного потока вводится непрямоугольная матрица целых чисел [aij], i = 1, …, m, j = 1, …, n.  Значения m и ni заранее не известны и вводятся из входного потока.
 * Сформировать новую матрицу, поместив в ее i-ую строку те элементы из i-ой строки исходной матрицы, количество цифр в записи которых превышает среднее количество цифр в записи всех элементов данной строки матрицы.
 * Исходную и полученную матрицы вывести в выходной поток с необходимыми комментариями.
 */

#include <stdlib.h>
#include <stdio.h>
#include "util.h"
#include "sort.h"

typedef struct int_vector {
        size_t len;
        int *vector;
} IntVector;

typedef struct int_matrix {
        int height;
        IntVector *matrix;
} IntMatrix;

void read_matrix(IntMatrix *matrix) {
        size_t i;
        size_t j;

        printf("Input matrix height> ");
        matrix->height = input_size_t();
        matrix->matrix = xmalloc(sizeof(IntVector) * matrix->height);

        for (i = 0; i < matrix->height; ++i) {
                printf("Input row width> ");
                matrix->matrix[i].len = input_size_t();
                matrix->matrix[i].vector = xmalloc(sizeof(int) * matrix->matrix[i].len);

                printf("Input row values (space separated)> ");
                for (j = 0; j < matrix->matrix[i].len; ++j) {
                        matrix->matrix[i].vector[j] = input_int();
                }
        }
}

void print_matrix(IntMatrix *matrix) {
        size_t i;
        size_t j;

        printf("Matrix of height %i:\n", matrix->height);

        for (i = 0; i < matrix->height; ++i) {
                printf("[i = %i, n = %i]: ", i, matrix->matrix[i].len);
                print_int_array(matrix->matrix[i].vector, matrix->matrix[i].len);
        }
}

int digit_count(int n) {
        return get_number_metadata(n).digit_count;
}

void filter_row(const IntVector *vector, IntVector *filtered_vector){
        int digit_count_sum = 0;
        size_t i;
        size_t fp;
        for (i = 0; i < vector->len; ++i) {
                digit_count_sum += digit_count(vector->vector[i]);
        }

        filtered_vector->len = 0;
        for (i = 0; i < vector->len; ++i) {
                if (digit_count(vector->vector[i]) * vector->len > digit_count_sum) {
                        filtered_vector->len++;
                }
        }

        filtered_vector->vector = xmalloc(sizeof(int) * filtered_vector->len);
        for (i = 0, fp = 0; i < vector->len; ++i) {
                if (digit_count(vector->vector[i]) * vector->len > digit_count_sum) {
                        filtered_vector->vector[fp++] = vector->vector[i];
                }
        }

}

void filter_matrix(const IntMatrix *matrix, IntMatrix *filtered_matrix) {
        size_t i;
        filtered_matrix->height = matrix->height;
        filtered_matrix->matrix = xmalloc(sizeof(IntVector) * matrix->height);

        for (i = 0; i < matrix->height; ++i){
                filter_row(&matrix->matrix[i], &filtered_matrix->matrix[i]);
        }
}

int main() {
        size_t i;
        struct int_matrix matrix;
        struct int_matrix filtered_matrix;

        read_matrix(&matrix);
        filter_matrix(&matrix, &filtered_matrix);

        puts("Original matrix:");
        print_matrix(&matrix);
        puts("Filtered matrix (only elements with above average digit count in rows are included):");
        print_matrix(&filtered_matrix);
        for (i = 0; i < matrix.height; ++i){
                free(matrix.matrix[i].vector);
                free(filtered_matrix.matrix[i].vector);
        }
        free(matrix.matrix);
        free(filtered_matrix.matrix);

        return 0;
}
