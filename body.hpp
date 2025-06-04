#pragma once
#include "vector.hpp"
#include <tuple>
class Body {
public:
    Body(const double& mass, const Vector& position, const Vector& velocity,
    double size = 8.0, std::tuple<double, double, double> color = {1, 1, 1});

    double mass;
    Vector position;
    Vector velocity;

    double size;
    std::tuple<double,double,double> color;
};
