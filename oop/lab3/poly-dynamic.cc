#include <algorithm>
#include <stdexcept>
#include <cmath>
#include <iostream>
#include "poly-dynamic.hpp"


DynamicPoly::DynamicPoly(std::vector<double> coeffs)
    : coeffs_ {coeffs}
{ trim(); }

double& DynamicPoly::operator[](size_t i) {
    if (i >= size()) {
        coeffs_.resize(i + 1);
    }
    return coeffs_[i];
}

double DynamicPoly::operator[](size_t i) const {
    if (i >= size()) { 
        return 0;
    }
    return coeffs_[i];
}

size_t DynamicPoly::size() const {
    return coeffs_.size();
}

void DynamicPoly::trim() {
    size_t max_non_zero_index = -1;

    for(ssize_t i = size() - 1; i >= 0; i--) {
        if (fabs(coeffs_[i]) > EPS) {
            max_non_zero_index = i;
            break;
        }
    }
    coeffs_.resize(max_non_zero_index + 1);
}

/* Addition */
DynamicPoly DynamicPoly::operator+(const DynamicPoly &poly2) const {
    DynamicPoly res;
    for(size_t i = 0; i < std::max(size(), poly2.size()); i++) {
        res[i] = (*this)[i] + poly2[i];
    }
    res.trim();
    return std::move(res);
}

DynamicPoly DynamicPoly::operator+(DynamicPoly &&poly2) {
    for(size_t i = 0; i < size(); i++) {
        poly2[i] += (*this)[i];
    }
    return std::move(poly2);
}

DynamicPoly& DynamicPoly::operator+=(const DynamicPoly &poly2) {
    for(size_t i = 0; i < poly2.size(); i++) {
        (*this)[i] += poly2[i];
    }
    return *this;
}

DynamicPoly DynamicPoly::operator+() const {
    return std::move(DynamicPoly((*this)));
}


/* Subtraction */
DynamicPoly DynamicPoly::operator-(const DynamicPoly &poly2) const {
    DynamicPoly res;
    for(size_t i = 0; i < std::max(size(), poly2.size()); i++) {
        res[i] = (*this)[i] - poly2[i];
    }
    res.trim();
    return std::move(res);
}

DynamicPoly DynamicPoly::operator-(DynamicPoly &&poly2) {
    for(size_t i = 0; i < std::max(size(), poly2.size()); i++) {
        poly2[i] = (*this)[i] - poly2[i];
    }
    poly2.trim();
    return std::move(poly2);
}

DynamicPoly& DynamicPoly::operator-=(const DynamicPoly &poly2) {
    for(size_t i = 0; i < std::max(size(), poly2.size()); i++) {
        (*this)[i] -= poly2[i];
    }
    trim();
    return *this;
}

DynamicPoly DynamicPoly::operator-() const {
    DynamicPoly res;
    for(size_t i = 0; i < size(); i++) {
        res[i] = -(*this)[i];
    }
    return std::move(res);
}

/* Shift (fast multiplication or division by x ** n) */
DynamicPoly DynamicPoly::operator<<(const size_t n) const {
    if (n == 0) {
        return *this;
    }

    DynamicPoly res;
    for(ssize_t i = size(); i >= 0; i--) {
        res[i + n] = (*this)[i];
    }
    return res;
}

DynamicPoly DynamicPoly::operator>>(const size_t n) const {
    if (n == 0) {
        return *this;
    }

    DynamicPoly res;
    for(ssize_t i = n; i < size(); i++) {
        res[i - n] = (*this)[i];
    }
    return res;
}


/* Multiplication */
DynamicPoly DynamicPoly::operator*(const DynamicPoly &poly2) const {
    DynamicPoly res;
    for(size_t i = 0; i < size(); i++) { 
        if (fabs((*this)[i]) <= EPS) {
            continue;
        }
        for(size_t j = 0; j < poly2.size(); j++) {
            res[i + j] += (*this)[i] * poly2[j]; 
        }
    }
    res.trim();
    return std::move(res);
}

DynamicPoly DynamicPoly::operator*(const double c) const {
    DynamicPoly res = *this;
    for(size_t i = 0; i < size(); i++) {
        res[i] *= c;
    }
    return std::move(res);
}



/* Division */
std::pair<DynamicPoly, DynamicPoly> DynamicPoly::divrem(const DynamicPoly &poly2) const {
    DynamicPoly div;
    DynamicPoly rem = *this;
    size_t max_non_zero_index = 0;

    for(size_t i = poly2.size() - 1; i > 0; i--) {
        if (fabs(poly2[i]) > EPS) {
            max_non_zero_index = i;
            break;
        }
    }

    if (fabs(poly2[max_non_zero_index]) < EPS) {
        throw std::logic_error("Division by zero.");
    }

    for(size_t i = size() - 1; i >= max_non_zero_index; i--) {
        div[i - max_non_zero_index] = rem[i] / poly2[max_non_zero_index];
        rem -= (poly2 << (i - max_non_zero_index)) *
            (div[i - max_non_zero_index]);
    }
    div.trim();
    rem.trim();

    return std::make_pair(
            div,
            rem
            );
}

std::pair<DynamicPoly, DynamicPoly> DynamicPoly::div_by_x_minus_const(const double c) {
    return divrem(DynamicPoly(std::vector<double>({1, -c})));
}

std::string DynamicPoly::to_string() const {
    std::string res;
    for(ssize_t i = size() - 1; i >= 0; i--) { 
        if (fabs((*this)[i]) > EPS) {
            if (res.length() != 0) {
                res += " + ";
            }
            if (i != 0) {
                if (fabs((*this)[i] - 1) > EPS) {
                    res += std::to_string((*this)[i]) + " * ";
                }
                res += "x ** " + std::to_string(i);
            } else {
                res += std::to_string((*this)[i]);
            }
        }
    }
    if (res.length()) {
        return res;
    } else {
        return "0";
    }
}
