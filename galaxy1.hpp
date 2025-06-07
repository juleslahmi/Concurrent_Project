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

private:
    // Struct for thread-local force storage
    struct ThreadData {
        std::vector<Vector> forces;
    };

    void computeForcesForThread(int n, int start, int end, std::vector<Vector>& forces);
    void launchThreads(const int n, const int nb_pairs, int num_threads, std::vector<ThreadData>& thread_data);
};

// Helper function: Convert flat pair index to (i, j) pair
std::pair<int, int> indexToPair(int k, int n);