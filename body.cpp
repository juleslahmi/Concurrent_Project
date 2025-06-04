#include "body.hpp"
#include <tuple>

Body::Body(const double& mass, const Vector& position, const Vector& velocity,
           double size, std::tuple<double,double,double> color)
    : mass(mass), position(position), velocity(velocity),size(size), color(color){}