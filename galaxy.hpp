#pragma once
#include <vector>
#include <thread>
#include "body.hpp"

class Galaxy {
public:
    static constexpr double G = 6.67430e-11;

    Galaxy(std::vector<Body*>& bodies);
    void simulate(double timestep, int num_threads);

    std::vector<Body*> bodies;
};

// Helpers
std::pair<int, int> indexToPair(int k, int n);
void ComputeForces(int start, int end, const std::vector<Body*>& bodies, std::vector<Vector>& forceOutput);
