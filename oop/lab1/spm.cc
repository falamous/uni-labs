#include <iostream>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <exception>
#include "spm.h"

std::unordered_map<int, int>& SparseMatrix::get_line(int i) {
    return matrix[i];
}

void SparseMatrix::add_point(int i, int j, int value) { 
    matrix[i][j] = value;
    i_min = std::min(i_min, i);
    i_max = std::max(i_max, i + 1);
    j_min = std::min(j_min, j);
    j_max = std::max(j_max, j + 1);
}

std::istream& operator>>(std::istream &is, SparseMatrix &matrix) {
    matrix.i_min = 0;
    matrix.j_min = 0;
    is >> matrix.i_max;
    is >> matrix.j_max;

    if (is.eof()) {
        throw std::logic_error("Unexpected EOF.");
    }
    if (is.bad()) {
        throw std::logic_error("Could not parse size.");
    }

    for (;;) {
        int i, j, value;

        is >> i;
        if (is.eof()) { break; }
        if (is.bad()) { throw std::logic_error("Could not parse row."); }
        is >> j;
        if (is.eof()) { break; }
        if (is.bad()) { throw std::logic_error("Could not parse row."); }
        is >> value;
        if (is.bad()) { throw std::logic_error("Could not parse row."); }

        matrix.add_point(i, j, value);
        if (is.eof()) { break; }
    }

    return is;
}
