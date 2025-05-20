#include<iostream>
#include <algorithm>

#include <string>
#include <stdio.h>

#include <vector>
#include <cmath>

double square(double x) {return x * x;}

class Vector {
    public:
        explicit Vector(double x = 0, double y = 0) {
            data[0] = x;
            data[1] = y;
        }
        double norm2() const {
            return data[0] * data[0] + data[1] * data[1];
        }
        double norm() const {
            return sqrt(norm2());
        }
        void normalize() {
            double n = norm();
            data[0] /= n;
            data[1] /= n;
        }
        double operator[](int i) const { return data[i]; };
        double& operator[](int i) { return data[i]; };
        double data[2];
    };

 
Vector operator+(const Vector& a, const Vector& b) {
    return Vector(a[0] + b[0], a[1] + b[1]);
}
Vector operator-(const Vector& a, const Vector& b) {
    return Vector(a[0] - b[0], a[1] - b[1]);
}
Vector operator*(const double a, const Vector& b) {
    return Vector(a*b[0], a*b[1]);
}
Vector operator*(const Vector& a, const double b) {
    return Vector(a[0]*b, a[1]*b);
}
Vector operator*(const Vector& a, const Vector& b) {
    return Vector(a[0] * b[0], a[1] * b[1]);
}
Vector operator/(const Vector& a, const double b) {
    return Vector(a[0] / b, a[1] / b);
}
double dot(const Vector& a, const Vector& b) {
    return a[0] * b[0] + a[1] * b[1];
}


class Body{
public:
    Body(const double& mass, Vector& position, Vector& velocity) : mass(mass), position(position), velocity(velocity) {}
    double mass;
    Vector position;
    Vector velocity;
};

class Galaxy{
public:
    static constexpr double G = 6.67430e-11;
    std::vector<Body*> bodies;
    Galaxy(std::vector<Body*>& bodies) : bodies(bodies) {}

    void simulate(double timestep){
        std::vector<Vector> forces(bodies.size(), Vector(0, 0));

        for(int i = 0; i < bodies.size(); i++){
            for(int j = i+1; j < bodies.size(); j++){
                if (i == j) continue;
                double a1 = square(bodies[i]->position[0] - bodies[j]->position[0]);
                double a2 = square(bodies[i]->position[1] - bodies[j]->position[1]);

                double f_ij = G * bodies[i]->mass * bodies[j]->mass / (a1 + a2);
                Vector dir = bodies[j]->position - bodies[i]->position;

                Vector force = f_ij * dir;
                forces[i] = forces[i] + force;
                forces[j] = forces[j] - force;
            }
        }
        
        for (size_t i = 0; i < bodies.size(); ++i) {
            Vector acceleration = forces[i] / bodies[i]->mass;
            bodies[i]->velocity = bodies[i]->velocity + timestep * acceleration;
            bodies[i]->position = bodies[i]->position + timestep * bodies[i]->velocity;
        }
    }

};

int main() {

    double earthMass = 5.972e24;
    double moonMass = 7.348e22;
    double distance = 3.844e8;
    double G = 6.67430e-11;

    double orbitalVelocity = std::sqrt(G * earthMass / distance);

    Vector earthPos(0, 0), earthVel(0, 0);
    Vector moonPos(distance, 0);
    Vector moonVel(0, orbitalVelocity); 

    Body* earth = new Body(earthMass, earthPos, earthVel);
    Body* moon = new Body(moonMass, moonPos, moonVel);

    std::vector<Body*> bodies = { earth, moon };
    Galaxy galaxy(bodies);

    double timestep = 60;
    int steps = 24000;

    for(int i = 0; i < steps; i++){
        galaxy.simulate(timestep);
        std::cout << "Moon at (" << moon->position[0] << ", " << moon->position[1] << ")\n";
    }



    // Free memory
    for (auto& body : bodies) delete body;

    return 0;
}
