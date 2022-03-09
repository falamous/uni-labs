#include <cmath>
#include "gtest/gtest.h"
#include "../poly-static.hpp"
#include <iostream>


TEST(StaticTestSuite, coeffs) {
    StaticPoly<40> p(std::array<double, 6>({1, 2, 3, 0, INFINITY, NAN}));
    EXPECT_DOUBLE_EQ(p[0], 1);
    EXPECT_DOUBLE_EQ(p[1], 2);
    EXPECT_DOUBLE_EQ(p[2], 3);
    EXPECT_DOUBLE_EQ(p[3], 0);
    EXPECT_DOUBLE_EQ(p[4], INFINITY);
    EXPECT_NE(p[5], p[5]);
    p[7] = 1337;
    EXPECT_EQ(p[7], 1337);

}

TEST(StaticTestSuite, addition) {
    StaticPoly<40> p(std::array<double, 4>({1, 2, 3, 4}));
    StaticPoly<40> q(std::array<double, 4>({43, 23, 421, 5}));

    auto a = p + q;
    for(size_t i = 0; i < 40; i++) {
        EXPECT_DOUBLE_EQ(a[i], p[i] + q[i]);
    }

    auto b = StaticPoly<40>(std::array<double, 4>({43, 23, 421, 5})) + p;
    for(size_t i = 0; i < 40; i++) {
        EXPECT_DOUBLE_EQ(b[i], p[i] + q[i]);
    }

    StaticPoly<40> pc(std::array<double, 4>({1, 2, 3, 4}));
    pc += q;
    for(size_t i = 0; i < 40; i++) {
        EXPECT_DOUBLE_EQ(pc[i], p[i] + q[i]);
    }
}

TEST(StaticTestSuite, mult_by_c) {
    StaticPoly<40> p(std::array<double, 6>({1, 2, 3, 4, 43,5234}));

    auto a = p * 1337;
    for(size_t i = 0; i < 40; i++) {
        EXPECT_DOUBLE_EQ(a[i], p[i] * 1337);
    }
    auto pc = StaticPoly<40>(std::array<double, 6>({1, 2, 3, 4, 43,5234})) * 1337;
    for(size_t i = 0; i < 40; i++) {
        EXPECT_DOUBLE_EQ(pc[i], p[i] * 1337);
    }

}

TEST(StaticTestSuite, mult) {
    StaticPoly<40> p(std::array<double, 6>({1, 2, 3, 4, 43,5234}));
    StaticPoly<40> q(std::array<double, 5>({1, 2432,432, 432,756}));
    auto res = p * q;
    StaticPoly<40> true_res(std::array<double, 11>({1, 2434, 5299, 8596, 12687, 114346, 12751660, 2282688, 2293596, 3956904}));

    for(size_t i = 0; i < 40; i++) {
        EXPECT_DOUBLE_EQ(res[i], true_res[i]);
    }
}


TEST(StaticTestSuite, mult_and_div) {
    StaticPoly<40> p(std::array<double, 10>({214, 279, 71, 84, 73, 27, 220, 217, 57, 259}));
    StaticPoly<40> q(std::array<double, 7>({55, 195, 117, 215, 134, 113, 110}));
    auto divrem_res = (p * q).divrem(q);
    for(size_t i = 0; i < 40; i++) {
        EXPECT_DOUBLE_EQ(divrem_res.second[i], 0);
    }

    for(size_t i = 0; i < 40; i++) {
        EXPECT_DOUBLE_EQ(divrem_res.first[i], p[i]);
    }
}
