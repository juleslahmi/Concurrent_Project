#pragma once
#include "vector.hpp"

class Body {
public:
    Body(const double& mass, const Vector& position, const Vector& velocity);

    double mass;
    Vector position;
    Vector velocity;
};
