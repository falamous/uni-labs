#include <cmath>
#include "gtest/gtest.h"
#include "../deltoid.h"

TEST(DeltiodTestSuite, radius){
    Deltoid d1(1);
    Deltoid d2(
            std::pair<double, double>(0, 0),
            std::pair<double, double>(6, 8)
            );
    EXPECT_DOUBLE_EQ(d1.get_radius(), 1);
    EXPECT_DOUBLE_EQ(d2.get_radius(), 5);
}

TEST(DeltiodTestSuite, tangent_line_length){
    Deltoid d(1337);
    EXPECT_DOUBLE_EQ(d.tangent_line_length(), 4 * 1337);
}

TEST(DeltiodTestSuite, length){
    Deltoid d(31337);
    EXPECT_DOUBLE_EQ(d.length(), 16 * 31337);
}
TEST(DeltiodTestSuite, area){
    Deltoid d(17171);
    EXPECT_DOUBLE_EQ(d.area(), 2 * M_PI * 17171);
}

TEST(DeltiodTestSuite, point_by_t){
    Deltoid d(1);
    EXPECT_DOUBLE_EQ(d.point_by_t(0).first, 3);
    EXPECT_DOUBLE_EQ(d.point_by_t(0).second, 0);
}

TEST(DeltiodTestSuite, center) {
    Deltoid d1(
            17,
            std::pair<double, double>(1337, 432)
            );
    Deltoid d2(
            17,
            std::pair<double, double>(17, 867)
            );
    EXPECT_DOUBLE_EQ(d1.get_radius(), d2.get_radius());
    EXPECT_DOUBLE_EQ(d1.area(), d2.area());
    EXPECT_DOUBLE_EQ(d1.length(), d2.length());
    EXPECT_DOUBLE_EQ(d1.tangent_line_length(), d2.tangent_line_length());
    EXPECT_DOUBLE_EQ(d1.point_by_t(31337).first - d2.point_by_t(31337).first, 1337 - 17);
    EXPECT_DOUBLE_EQ(d1.point_by_t(31337).second - d2.point_by_t(31337).second, 432 - 867);
}

TEST(DeltiodTestSuite, rotation) {
    Deltoid d1(
            17,
            std::pair<double, double>(0, 0),
            0.4123
            );
    Deltoid d2(
            17,
            std::pair<double, double>(0, 0),
            0.77542432
            );
    EXPECT_DOUBLE_EQ(d1.get_radius(), d2.get_radius());
    EXPECT_DOUBLE_EQ(d1.area(), d2.area());
    EXPECT_DOUBLE_EQ(d1.length(), d2.length());
    EXPECT_DOUBLE_EQ(d1.tangent_line_length(), d2.tangent_line_length());
}

TEST(DeltiodTestSuite, two_centers) {
    Deltoid d1(5);
    Deltoid d2(
            std::pair<double, double>(133, 1337),
            std::pair<double, double>(133 + 8, 1337 + 6)
            );
    EXPECT_DOUBLE_EQ(d1.get_radius(), d2.get_radius());
    EXPECT_DOUBLE_EQ(d1.area(), d2.area());
    EXPECT_DOUBLE_EQ(d1.length(), d2.length());
    EXPECT_DOUBLE_EQ(d1.tangent_line_length(), d2.tangent_line_length());
}
