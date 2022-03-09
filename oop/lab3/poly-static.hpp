#include <array>
#include <algorithm>
#include <stdexcept>
#include <cstddef>
#include <cmath>
#include <iostream>

static const double EPS = 1e-12;

template<size_t N=30>
class StaticPoly {
    std::array<double, N> coeffs_;

    public:
    template<size_t n>
    StaticPoly(std::array<double, n> coeffs) {
        static_assert(n <= N, "Trying to create a poly with more coeffs than allowed.");
        coeffs_ = {0};
        for(size_t i = 0; i < n; i++) {
            coeffs_[i] = coeffs[i];
        }
    }

    StaticPoly()
        : coeffs_ {0}
    {}

    double& operator[](size_t i) {
        return coeffs_[i];
    }

    double operator[](size_t i) const {
        return coeffs_[i];
    }

    /* Addition */
    StaticPoly operator+(const StaticPoly &poly2) const {
        StaticPoly res;
        for(size_t i = 0; i < N; i++) {
            res[i] = coeffs_[i] + poly2[i];
        }
        return std::move(res);
    }

    StaticPoly operator+(StaticPoly &&poly2) {
        for(size_t i = 0; i < N; i++) {
            poly2[i] += coeffs_[i];
        }
        return std::move(poly2);
    }

    StaticPoly& operator+=(const StaticPoly &poly2) {
        for(size_t i = 0; i < N; i++) {
            coeffs_[i] += poly2[i];
        }
        return *this;
    }

    StaticPoly operator+() const {
        return std::move(StaticPoly(coeffs_));
    }


    /* Subtraction */
    StaticPoly operator-(const StaticPoly &poly2) const {
        StaticPoly res;
        for(size_t i = 0; i < N; i++) {
            res[i] = coeffs_[i] - poly2[i];
        }
        return std::move(res);
    }

    StaticPoly operator-(StaticPoly &&poly2) {
        for(size_t i = 0; i < N; i++) {
            poly2[i] = coeffs_[i] - poly2[i];
        }
        return std::move(poly2);
    }

    StaticPoly& operator-=(const StaticPoly &poly2) {
        for(size_t i = 0; i < N; i++) {
            coeffs_[i] -= poly2[i];
        }
        return *this;
    }

    StaticPoly operator-() const {
        StaticPoly res;
        for(size_t i = 0; i < N; i++) {
            res[i] = -coeffs_[i];
        }
        return std::move(res);
    }

    /* Shift (fast multiplication or division by x ** n) */

    StaticPoly operator<<(const size_t n) const {
        if (n == 0) {
            return *this;
        }
        StaticPoly res;
        if (n > N) {
            throw std::runtime_error("Polynom shift exceeds static array size.");
        }
        for(ssize_t i = N - 1; i >= std::min(n, N - n); i--) {
            if (fabs(coeffs_[i]) > EPS && (i >= N - n)) {
                throw std::runtime_error("Polynom shift exceeds static array size.");
            }
            if (i >= n) {
                res[i] = coeffs_[i - n];
            }
        }
        res[0] = 0;
        return res;
    }

    StaticPoly operator>>(const size_t n) const {
        StaticPoly res;
        for(ssize_t i = 0; i < N - n; i++) {
            res[i] = coeffs_[i + n];
        }
        for(ssize_t i = std::min((ssize_t)(N - n), (ssize_t)0); i < N - 1; i++) {
            res[i] = 0;
        }
        return res;
    }


    /* Multiplication */

    StaticPoly operator*(const StaticPoly &poly2) const {
        StaticPoly res;
        for(size_t i = 0; i < N; i++) { 
            if (fabs(coeffs_[i]) <= EPS) {
                continue;
            }
            for(size_t j = 0; j < N; j++) {
                if (i + j >= N) {
                    if (fabs(poly2[j]) > EPS) {
                        throw std::runtime_error("Multiple of 2 polynoms does not fit in static array.");
                    }
                } else {
                    res[i + j] += coeffs_[i] * poly2[j]; 
                }
            }
        }
        return std::move(res);
    }

    StaticPoly operator*(const double c) const {
        StaticPoly res = *this;
        for(size_t i = 0; i < N; i++) {
            res[i] *= c;
        }
        return std::move(res);
    }

    /* Division */

    std::pair<StaticPoly, StaticPoly> divrem(const StaticPoly &poly2) const {
        StaticPoly div;
        StaticPoly rem = *this;
        size_t max_non_zero_index = 0;

        for(size_t i = N - 1; i > 0; i--) {
            if (fabs(poly2[i]) > EPS) {
                max_non_zero_index = i;
                break;
            }
        }

        if (fabs(poly2[max_non_zero_index]) < EPS) {
            throw std::logic_error("Division by zero.");
        }

        for(size_t i = N - 1; i >= max_non_zero_index; i--) {
            div[i - max_non_zero_index] = rem[i] / poly2[max_non_zero_index];
            rem -= (poly2 << (i - max_non_zero_index)) *
                (div[i - max_non_zero_index]);
        }

        return std::make_pair(
                div,
                rem
                );
    }

    std::pair<StaticPoly, StaticPoly> div_by_x_minus_const(const double c) {
        return divrem(StaticPoly(std::array<double, 2>({1, -c})));
    }

    std::string to_string() const {
        std::string res;
        for(ssize_t i = N - 1; i >= 0; i--) { 
            if (fabs(coeffs_[i]) > EPS) {
                if (res.length() != 0) {
                    res += " + ";
                }
                if (i != 0) {
                    if (fabs(coeffs_[i] - 1) > EPS) {
                        res += std::to_string(coeffs_[i]) + " * ";
                    }
                    res += "x ** " + std::to_string(i);
                } else {
                    res += std::to_string(coeffs_[i]);
                }
            }
        }
        if (res.length()) {
            return res;
        } else {
            return "0";
        }
    }
};
