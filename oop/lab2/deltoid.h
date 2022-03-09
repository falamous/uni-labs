#include <utility>
#include <string>


class Deltoid {
    double radius_;
    std::pair<double, double> center_;
    double rotation_;

    std::pair<double, double> apply_transformations(
        std::pair<double, double> coords
        ) const;

    void ensure_valid_arguments();
    std::string format_formula(const std::pair<std::string, std::string> &coords, const std::string &radius) const;
    std::pair<std::string, std::string> format_coord_transformations() const;

    public:

    Deltoid(
            double radius,
            std::pair<double, double> center={0, 0},
            double rotation=0
            );

    Deltoid(
            std::pair<double, double> center,
            std::pair<double, double> second_center
            );


    double get_radius() const;
    void set_radius(double radius);

    double get_rotation();
    void set_rotation(double rotation);

    std::pair<double, double> get_center();
    void set_center(std::pair<double, double> center);


    double tangent_line_length() const;
    double length() const;
    double area() const;
    std::pair<double, double> point_by_t(double t) const;

    std::string shortened_formula() const;
    std::string full_formula() const;
};
