#include <string>
#include <gtest/gtest.h>
#include "../spm.h"

#include <iostream>

TEST(SparseMatrix, SparseMatrix) {
    SparseMatrix spm;

    std::stringstream ss("0 0\n 3 4123 412");
    ss >> spm;


    for(auto lv: spm.get_line(3)) {

        EXPECT_EQ(lv.first, 4123);
        EXPECT_EQ(lv.second, 412);
    }
    spm.add_point(1, 2, 32);
    for(auto lv: spm.get_line(1)) {

        EXPECT_EQ(lv.first, 2);
        EXPECT_EQ(lv.second, 32);
    }
    spm.add_point(1, 4, 6);

    EXPECT_EQ(spm.sum_line_if(1, [](int a) -> bool { return a % 2 == 0; }), 38);
    EXPECT_EQ(spm.get_2_pred_diff_sum_list(
                [](int a) -> bool { return a % 2 == 0; },
                [](int a) -> bool { return a < 10; }
                ), 
            std::vector<int>({0, 32, 0, 412})
            );
}
