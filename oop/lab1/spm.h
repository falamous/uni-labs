#include <iostream>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <exception>

class SparseMatrix {
    std::unordered_map<int, std::unordered_map<int, int>> matrix;
    int i_min, i_max, j_min, j_max;
    public:

    std::unordered_map<int, int>& get_line(int i);
    template <class UnaryPredicate1, class UnaryPredicate2>
    std::vector<int> get_2_pred_diff_sum_list(
            UnaryPredicate1 pred1,
            UnaryPredicate2 pred2
            );
    void add_point(int i, int j, int value);
    template <class UnaryPredicate>
        intmax_t sum_line_if(int i, UnaryPredicate pred);
    friend std::istream& operator>>(std::istream &is, SparseMatrix &matrix);
    bool operator==(const SparseMatrix &spm) const;
};

template <class UnaryPredicate1, class UnaryPredicate2>
std::vector<int> SparseMatrix::get_2_pred_diff_sum_list(
        UnaryPredicate1 pred1,
        UnaryPredicate2 pred2
        ) {
    std::vector<int> b;
    for(int i = i_min; i < i_max; i++) {
        b.push_back(sum_line_if(i, pred1) - sum_line_if(i, pred2));
    }
    return b;
}



template <class UnaryPredicate>
intmax_t SparseMatrix::sum_line_if(int i, UnaryPredicate pred) {
    intmax_t sum = 0;
    for(auto value: get_line(i)) {
        if (pred(value.second)) {
            sum += value.second;
        }
    }
    return sum;
}
