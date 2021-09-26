/* 
 * Вариант 35
 * Дана последовательность из n натуральных чисел.
 * Для каждого числа исходной последовательности подсчитать количество и сумму цифр в записи числа.
 * Сформировать две новых последовательности, состоящие соответственно из количества и суммы цифр в записи чисел, стоящих на нечетных местах в исходной последовательности. Упорядочить полученные последовательности по возрастанию. Вывести исходную и полученные последовательности на экран.
 * В программе использовать функцию, которая определяет количество и сумму цифр в записи числа. 
 */

#include <stdlib.h>
#include <stdio.h>
#include "util.h"
#include "sort.h"

int compare_int(const void *a, const void *b) {
        /* compar function for qsort and self writen mergesort */
        int na = *(int *)a;
        int nb = *(int *)b;

        /* Can't simple return na - nb, because for example INT_MAX, -1 would cause overflow. */
        if (na > nb) {
                return 1;
        }
        if (na == nb) {
                return 0;
        }
        if (na < nb) {
                return -1;
        }
}

int main() {
        size_t numbers_length;
        size_t i;
        NumberMetadata current_number;
        int *odd_number_digit_counts;
        int *odd_number_digit_sums;
        int *numbers;

        printf("Input length of the numbers array> ");
        numbers_length = input_size_t();
        numbers = xmalloc(sizeof(int) * numbers_length);
        odd_number_digit_counts = xmalloc(sizeof(int) * (numbers_length / 2));
        odd_number_digit_sums = xmalloc(sizeof(int) * (numbers_length / 2));

        printf("Input integers separated by whitespace characters> ");
        for (i = 0; i < numbers_length; i++) {
                numbers[i] = input_int();
                current_number = get_number_metadata(numbers[i]);

                if (i % 2 == 1) {
                        odd_number_digit_counts[i / 2] = current_number.digit_count;
                        odd_number_digit_sums[i / 2] = current_number.digit_sum;
                }
        }

        mergesort(odd_number_digit_counts,
                        numbers_length/2, sizeof(int), compare_int);
        mergesort(odd_number_digit_sums,
                        numbers_length/2, sizeof(int), compare_int);

        printf("Original sequence of length %zu:\n", numbers_length);
        print_int_array(numbers, numbers_length);
        printf("Sorted sequence of odd number digits counts of length %zu:\n", numbers_length / 2);
        print_int_array(odd_number_digit_counts, numbers_length / 2);
        printf("Sorted sequence of odd number digits sums of length %zu:\n", numbers_length / 2);
        print_int_array(odd_number_digit_sums, numbers_length / 2);

        return 0;
}
