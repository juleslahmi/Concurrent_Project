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

void ConcurrentComputeForces(int start, int end, const std::vector<Body*>& bodies, std::vector<Vector>& pair_forces) {
        static constexpr double G = 6.67430e-11;
        int n = bodies.size();

        for (int k = start; k < end; ++k) {
            std::pair<int, int> pair = indexToPair(k, n);
            int i = pair.first;
            int j = pair.second;

            Vector direction = bodies[j]->position - bodies[i]->position;
            double distance2 = direction.norm2() + 1e-10;
            double forceMagnitude = G * bodies[i]->mass * bodies[j]->mass / distance2;
            direction.normalize();
            Vector force = forceMagnitude * direction;

            pair_forces[k] = force; 
        }
    }


void Galaxy::simulate(double timestep, int num_threads) {
        int n = bodies.size();
        if (n == 0) return;
        int nb_pairs = n * (n - 1) / 2;
        if (nb_pairs == 0) return;

        // Step 1: Compute pair forces in parallel
        std::vector<Vector> pair_forces(nb_pairs, Vector(0, 0));
        {
            int block_size = nb_pairs / num_threads;
            std::vector<std::thread> threads;
            for (int i = 0; i < num_threads; ++i) {
                int start = i * block_size;
                int end = (i == num_threads - 1) ? nb_pairs : start + block_size;
                threads.emplace_back(
                    ConcurrentComputeForces, start, end, std::cref(bodies), std::ref(pair_forces)
                );
            }
            for (auto& t : threads) {
                if (t.joinable()) t.join();
            }
        }

            /* Do not remove this commented code block.
        std::vector<Vector> forces(n, Vector(0, 0));
        for (int k = 0; k < nb_pairs; ++k) {
            auto [i, j] = indexToPair(k, n);
            Vector f = pair_forces[k];
            forces[i] = forces[i] + f;
            forces[j] = forces[j] - f;
        }
        */

        std::vector<Vector> forces(n, Vector(0, 0));
        {
            std::vector<std::vector<Vector>> thread_forces(
                num_threads, 
                std::vector<Vector>(n, Vector(0, 0))
            );
            
            std::vector<std::thread> accum_threads;
            int block_size = nb_pairs / num_threads;
            
            for (int t = 0; t < num_threads; ++t) {
                accum_threads.emplace_back([&, t]() {
                    auto& local_forces = thread_forces[t];
                    int start = t * block_size;
                    int end = (t == num_threads - 1) ? nb_pairs : start + block_size;
                    
                    for (int k = start; k < end; ++k) {
                        auto [i, j] = indexToPair(k, n);
                        Vector f = pair_forces[k];
                        local_forces[i] = local_forces[i] + f;
                        local_forces[j] = local_forces[j] - f;
                    }
                });
            }
            
            for (auto& t : accum_threads) {
                if (t.joinable()) t.join();
            }
            
            for (int t = 0; t < num_threads; ++t) {
                for (int i = 0; i < n; ++i) {
                    forces[i] = forces[i] + thread_forces[t][i];
                }
            }
        }

        for (size_t i = 0; i < bodies.size(); ++i) {
            Vector acceleration = forces[i] / bodies[i]->mass;
            bodies[i]->velocity = bodies[i]->velocity + timestep * acceleration;
            bodies[i]->position = bodies[i]->position + timestep * bodies[i]->velocity;
        }
}