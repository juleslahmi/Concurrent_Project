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

void Galaxy::computeForcesForThread(int n, int start, int end, std::vector<Vector>& forces) {
    constexpr double G = 6.67430e-11;
    for (int k = start; k < end; ++k) {
        const auto [i, j] = indexToPair(k, n);
        Vector direction = bodies[j]->position - bodies[i]->position;
        double distance2 = direction.norm2() + 1e-10;
        double forceMagnitude = G * bodies[i]->mass * bodies[j]->mass / distance2;
        direction.normalize();
        Vector force = forceMagnitude * direction;
        forces[i] = forces[i] + force;
        forces[j] = forces[j] - force;
    }
}

void Galaxy::launchThreads(const int n, const int nb_pairs, int num_threads, 
                          std::vector<ThreadData>& thread_data) {
    const int pairs_per_thread = (nb_pairs + num_threads - 1) / num_threads;
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    for (int t = 0; t < num_threads; ++t) {
        const int start = t * pairs_per_thread;
        const int end = std::min(start + pairs_per_thread, nb_pairs);
        threads.emplace_back([this, n, start, end, &td = thread_data[t]]() {
            computeForcesForThread(n, start, end, td.forces);
        });
    }

    for (auto& t : threads) t.join();
}

void Galaxy::simulate(double timestep, int num_threads) {
    const int n = bodies.size();
    if (n == 0) return;
    const int nb_pairs = n * (n - 1) / 2;
    if (nb_pairs == 0) return;

    std::vector<ThreadData> thread_data(num_threads);
    for (auto& td : thread_data) {
        td.forces.resize(n, Vector(0, 0));
    }

    launchThreads(n, nb_pairs, num_threads, thread_data);

    std::vector<Vector> total_forces(n, Vector(0, 0));
    for (int i = 0; i < n; ++i) {
        for (int t = 0; t < num_threads; ++t) {
            total_forces[i] = total_forces[i] + thread_data[t].forces[i];
        }
    }
    
    for (int i = 0; i < n; ++i) {
        Vector acceleration = total_forces[i] / bodies[i]->mass;
        bodies[i]->velocity = bodies[i]->velocity + (timestep * acceleration);
        bodies[i]->position = bodies[i]->position + (timestep * bodies[i]->velocity);
    }
}