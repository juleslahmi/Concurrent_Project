#include "galaxy.hpp"
#include <algorithm>  // For std::min

// Constructor
Galaxy::Galaxy(std::vector<Body*>& bodies)
    : bodies(bodies) {}

// Helper: Convert flat pair index to (i, j) pair
std::pair<int, int> indexToPair(int k, int n) {
    int i = 0;
    while (k >= n - i - 1) {
        k -= (n - i - 1);
        ++i;
    }
    int j = i + 1 + k;
    return std::make_pair(i, j);
}

// Compute gravitational forces for a block of pairs
void ComputeForces(int start, int end, const std::vector<Body*>& bodies, std::vector<Vector>& forceOutput) {
    static constexpr double G = 6.67430e-11;
    int n = bodies.size();
    std::vector<Vector> local_forces(n, Vector(0, 0));

    for (int k = start; k < end; ++k) {
        std::pair<int, int> pair = indexToPair(k, n);
        int i = pair.first;
        int j = pair.second;

        Vector direction = bodies[j]->position - bodies[i]->position;
        double distance2 = direction.norm2() + 1e-10;  // Avoid division by zero
        double forceMagnitude = G * bodies[i]->mass * bodies[j]->mass / distance2;
        direction.normalize();
        Vector force = forceMagnitude * direction;

        local_forces[i] = local_forces[i] + force;
        local_forces[j] = local_forces[j] - force;
    }

    for (int i = 0; i < n; ++i) {
        forceOutput[i] = forceOutput[i] + local_forces[i];
    }
}

// Main simulation function
void Galaxy::simulate(double timestep, int num_threads) {
    int n = bodies.size();
    int nb_pairs = n * (n - 1) / 2;

    std::vector<Vector> forces(n, Vector(0, 0));
    std::vector<std::vector<Vector>> thread_forces(num_threads, std::vector<Vector>(n, Vector(0, 0)));
    std::vector<std::thread> threads(num_threads);

    int block_size = nb_pairs / num_threads;

    // Launch threads to compute forces
    for (int i = 0; i < num_threads; ++i) {
        int start = i * block_size;
        int end = (i == num_threads - 1) ? nb_pairs : std::min(start + block_size, nb_pairs);
        threads[i] = std::thread(ComputeForces, start, end, std::cref(bodies), std::ref(thread_forces[i]));
    }

    // Wait for threads to finish
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    // Merge all thread-local force vectors
    for (int t = 0; t < num_threads; ++t) {
        for (int i = 0; i < n; ++i) {
            forces[i] = forces[i] + thread_forces[t][i];
        }
    }

    // Update positions and velocities
    for (int i = 0; i < n; ++i) {
        Vector acceleration = forces[i] / bodies[i]->mass;
        bodies[i]->velocity = bodies[i]->velocity + timestep * acceleration;
        bodies[i]->position = bodies[i]->position + timestep * bodies[i]->velocity;
    }
}
