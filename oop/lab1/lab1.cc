#include <iostream>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <exception>
#include "spm.h"

int main() {
    SparseMatrix matrix;

    std::cout << "Input matrix (n, m, then i, j, value triples)" << std::endl;
    std::cin >> matrix;
    for(auto value: matrix.get_2_pred_diff_sum_list(
                [](int a) -> bool { return a % 2 == 0; },
                [](int a) -> bool { return a > 0; }
                )) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
}
