#include <vector>
#include <string>

static const double EPS = 1e-12;

class DynamicPoly {
    std::vector<double> coeffs_;

    public:
    DynamicPoly(std::vector<double> coeffs = {});

    double& operator[](size_t i);
    double operator[](size_t i) const;
    size_t size() const;
    void trim();


    /* Addition */
    DynamicPoly operator+(const DynamicPoly &poly2) const;
    DynamicPoly operator+(DynamicPoly &&poly2);
    DynamicPoly& operator+=(const DynamicPoly &poly2);
    DynamicPoly operator+() const;


    /* Subtraction */
    DynamicPoly operator-(const DynamicPoly &poly2) const;
    DynamicPoly operator-(DynamicPoly &&poly2);
    DynamicPoly& operator-=(const DynamicPoly &poly2);
    DynamicPoly operator-() const;


    /* Shift (fast multiplication or division by x ** n) */
    DynamicPoly operator<<(const size_t n) const;
    DynamicPoly operator>>(const size_t n) const;


    /* Multiplication */
    DynamicPoly operator*(const DynamicPoly &poly2) const;
    DynamicPoly operator*(const double c) const;


    /* Division */
    std::pair<DynamicPoly, DynamicPoly> divrem(const DynamicPoly &poly2) const;
    std::pair<DynamicPoly, DynamicPoly> div_by_x_minus_const(const double c);
    std::string to_string() const;
};
