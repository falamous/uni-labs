#include "deltoid.h"
#include <iostream>
#include <functional>
#include <vector>
#include <memory>

/* 
 * Вариант 20. Дельтоида
 * Разработать класс, определяющий кривую – дельтоиду.
 * Дельтоида – плоская алгебраическая кривая 4-го порядка, описываемая фиксированной точкой окружности радиуса r, катящейся по внутренней стороне окружности радиуса R = 3r.
 * 1) Определить состояние класса.
 * 2) Разработать необходимые конструкторы и методы получения и изменения параметров, определяющих кривую.
 * 3) Вернуть длину пересечения области, ограниченной дельтоидой, с любой ее касательной.
 * 4) Вернуть длину кривой.
 * 5) Вернуть площадь, ограниченную кривой.
 * 6) Вернуть значения координат x и y в зависимости от заданного параметра t.
 * 7) Вернуть текстовое представление уравнения дельтоиды в декартовой системе координат в виде строки (char[] или wchar_t[]).
 * Разработать диалоговую программу для тестирования класса.
 */

template<class T, class Pred>
static T read_type(std::istream &is, Pred pred) {
    T v;
    std::string dump_string;

    is >> v;
    while (!std::cout.good() || !pred(v)) {
        std::cout << "Try again" << std::endl;
        if (std::cout.eof()) {
            throw std::runtime_error("Got an eof while reading");
        }
        is >> dump_string;
        is >> v;
    }
    return v;
}

template<class T>
static T read_type(std::istream &is) {
    return read_type<T>(is, [](T _) -> bool { return true; });
}

int main() {
    int choice;
    std::shared_ptr<Deltoid> deltoid = nullptr;
     /*
     * Listen in, this here's called a smart point.
     * It automaticly frees the memory it points to when there are no references to it.
     * If you dare call this a leak in your code review, I will come to your home, wipe all of your computers and install Temple OS on them.
     * */

    std::cout << "How do you wish to input your curve?" << std::endl;
    std::cout << "1 - radius, center, rotation" << std::endl;
    std::cout << "2 - first center, second center" << std::endl;

    choice = read_type<int>(std::cin, [](int choice) -> bool {
            return choice == 1 || choice == 2;
            });

    if (choice == 1) {
        std::cout << "r:";
        std::cout.flush();
        double radius = read_type<double>(std::cin);

        std::cout << "x, y:";
        std::cout.flush();
        const std::pair<double, double> center = std::make_pair(
                read_type<double>(std::cin),
                read_type<double>(std::cin)
                );

        std::cout << "phi:";
        std::cout.flush();
        double rotation = read_type<double>(std::cin);

        deltoid = std::shared_ptr<Deltoid>(new Deltoid(radius, center, rotation));
    } else {
        std::cout << "x, y:";
        std::cout.flush();
        const std::pair<double, double> center = std::make_pair(
                read_type<double>(std::cin),
                read_type<double>(std::cin)
                );

        std::cout << "x1, y1:";
        std::cout.flush();
        std::pair<double, double> second_center = std::make_pair(
                read_type<double>(std::cin),
                read_type<double>(std::cin)
                );

        deltoid = std::shared_ptr<Deltoid>(new Deltoid(center, second_center));
    }

    std::vector< std::pair< std::string, std::function<void ()> > > menu = {
        { "Tangent line length.", [deltoid]() { std::cout << deltoid->tangent_line_length() << std::endl; }},
        { "Curve length.", [deltoid]() { std::cout << deltoid->length() << std::endl; }},
        { "Curve area.", [deltoid]() { std::cout << deltoid->area() << std::endl; }},
        { "Coordinates by t.", [deltoid]() {
                                               std::cout << "t:";
                                               std::cout.flush();
                                               double t = read_type<double>(std::cin);
                                               std::pair<double, double> point = deltoid->point_by_t(t);
                                               std::cout << point.first << ", " << point.second << std::endl;
                                           }},
        { "Get shortened curve formula.", [deltoid]() { std::cout << deltoid->shortened_formula() << std::endl; }},
        { "Get full curve formula.", [deltoid]() { std::cout << deltoid->full_formula() << std::endl; }},
        { "Exit.", [deltoid]() { exit(0); }},
    };

    for(;;) {
        auto deltoid_center = deltoid->get_center();
        std::cout << "Deltoid(" <<
            "r = " << deltoid->get_radius() <<
            ", xc = " << deltoid_center.first <<
            ", yc = " << deltoid_center.second <<
            ", phi = " << deltoid->get_rotation() <<
            ")" << std::endl;
        for(int i = 0; i < menu.size(); i++) {
            std::cout << i << ". " << menu[i].first << std::endl;
        }
        choice = read_type<int>(std::cin, [menu](int choice) -> bool {
                return 0 <= choice && choice < menu.size();
                });
        menu[choice].second();
    }
}
