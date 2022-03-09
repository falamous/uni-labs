#include "deltoid.h"
#include <cmath>
#include <stdexcept>
#include <cstring>
#include <memory>

static const double EPS = 1e-12L;

static double point_distance(
        std::pair<double, double> a,
        std::pair<double, double> b
        ) {
    return sqrt(
            (a.first - b.first) * (a.first - b.first) +
            (a.second - b.second) * (a.second - b.second)
            );
}

std::pair<double, double> Deltoid::apply_transformations(
        std::pair<double, double> coords
        ) const {
    /* apply shift */
    const double xs = coords.first + center_.first;
    const double ys = coords.second + center_.second;
    return std::make_pair(xs, ys);
    /* apply rotation */
    const double xr = xs * cos(-rotation_) + ys * sin(-rotation_);
    const double yr = - xs * sin(-rotation_) + ys * cos(-rotation_);
    return std::make_pair(xr, yr);
}
void Deltoid::ensure_valid_arguments() {
    if (radius_ < EPS) {
        throw std::logic_error("Degenerate curve or negative radius.");
    }

    if (rotation_ < 0) {
        rotation_ += 2 * M_PI;
    }

    if (rotation_ < 0 || rotation_ > 2 * M_PI) {
        throw std::logic_error("Invalid angle.");
    }
}


Deltoid::Deltoid(
        double radius,
        std::pair<double, double> center,
        double rotation
       )
    : radius_ {radius}
    , center_ {center}
    , rotation_ {rotation}
{ ensure_valid_arguments(); }

Deltoid::Deltoid(
        std::pair<double, double> center,
        std::pair<double, double> second_center
       )
    : center_ {center}
{ 
    radius_ = point_distance(center, second_center) / 2;
    rotation_ = atan(
            (center.second - second_center.second)
            / 
            (center.first - second_center.first)
            );
    ensure_valid_arguments();
}

double Deltoid::get_radius() const {
    return radius_;
}
void Deltoid::set_radius(double radius) {
    radius_ = radius;
}

double Deltoid::get_rotation() {
    return rotation_;
}
void Deltoid::set_rotation(double rotation) {
    rotation_ = rotation;
}

std::pair<double, double> Deltoid::get_center() {
    return center_;
}
void Deltoid::set_center(std::pair<double, double> center) {
    center_ = center;
}


double Deltoid::tangent_line_length() const {
    return 4 * radius_;
}

double Deltoid::length() const {
    return 16 * radius_;
} 

double Deltoid::area() const {
    return 2 * M_PI * radius_;
}

std::pair<double, double> Deltoid::point_by_t(double t) const {
    return apply_transformations(std::make_pair(
            2 * radius_ * cos(t) + radius_ * cos(2 * t), /* (b - a) * cos(t) + a * cos((b - a) / a * t) */
            2 * radius_ * sin(t) - radius_ * sin(2 * t)  /* (b - a) * sin(t) - a * sin((b - a) / a * t) */
            ));
}

std::string Deltoid::format_formula(const std::pair<std::string, std::string> &coords, const std::string &radius) const {
    const std::string x = coords.first;
    const std::string y = coords.second;

    if (
            strlen(x.c_str()) != x.length() ||
            strlen(y.c_str()) != y.length() ||
            strlen(radius.c_str()) != radius.length()
            ) {
        throw std::logic_error("Attempting to format with coordinate formulas with zero bytes.");
    }

    size_t output_buffer_size = x.length() * 4 + y.length() * 3 + radius.length() * 3 + 100;
    std::unique_ptr<char[]> output_buffer(new char[output_buffer_size]);
    /*
     * Listen in, this here's called a smart point.
     * It automaticly frees the memory it points to when its exists scope.
     * If you dare call this a leak in your code review, I will come to your home, wipe all of your computers and install Temple OS on them.
     * */
    
    /* "(x ** 2 + y ** 2) ** 2 + 18 * r ** 2 * (x ** 2 + y ** 2) - 27 * r ** 4 = 8 * r * (x ** 3 - 3 * x * y ** 2)" */
    snprintf(output_buffer.get(), output_buffer_size,
            "(%1$s ** 2 + %2$s ** 2) ** 2"
            " + 18 * %3$s ** 2 * (%1$s ** 2 + %2$s ** 2)"
            "- 27 * %3$s ** 4"
            "= 8 * %3$s * (%1$s ** 3 - 3 * %1$s * %2$s ** 2)",
            x.c_str(), y.c_str(), radius.c_str());

    return std::string(output_buffer.get(), strlen(output_buffer.get()));
}

std::pair<std::string, std::string> Deltoid::format_coord_transformations() const {
    const std::string angle = std::to_string(rotation_);

    return make_pair(
            "(x * cos(" +  angle  + ") + y * sin(" + angle + ") - " + std::to_string(center_.first) + ")",
            "(-x * cos(" +  angle  + ") + y * sin(" + angle + ") - " + std::to_string(center_.second) + ")"
            );
}

std::string Deltoid::shortened_formula() const {
    auto coord_strings = format_coord_transformations();
    return format_formula(std::make_pair("x'", "y'"), "r") +
        "; x' = " + coord_strings.first +
        "; y' = " + coord_strings.second +
        "; r = " + std::to_string(radius_);
}

std::string Deltoid::full_formula() const {
    return format_formula(format_coord_transformations(), std::to_string(radius_));
}
