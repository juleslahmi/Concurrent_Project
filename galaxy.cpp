#include "galaxy.hpp"
#include <algorithm>
#include <iostream>

Galaxy::Galaxy(std::vector<Body*>& bodies, int version)
    : bodies(bodies), version(version) {}

std::pair<int, int> indexToPair(int k, int n) {
    int i = 0;
    while (k >= n - i - 1) {
        k -= (n - i - 1);
        ++i;
    }
    int j = i + 1 + k;
    return std::make_pair(i, j);
}

void Galaxy::simulate(double timestep, int num_threads) {
    if (version == 0) {
        simulate_v0(timestep, num_threads);
    } else {
        simulate_v1(timestep, num_threads);
    }
}

void Galaxy::simulate_v0(double timestep, int num_threads) {
    int n = bodies.size();
    if (n == 0) return;
    int nb_pairs = n * (n - 1) / 2;
    if (nb_pairs == 0) return;

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

void Galaxy::simulate_v1(double timestep, int num_threads) {
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
    
    updateBodiesParallel(total_forces, timestep, num_threads);
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

void Galaxy::updateBodiesParallel(const std::vector<Vector>& forces, double timestep, int num_threads) {
    const int n = bodies.size();
    if (n == 0) return;

    if (num_threads <= 1) {
        for (int i = 0; i < n; ++i) {
            Vector acceleration = forces[i] / bodies[i]->mass;
            bodies[i]->velocity = bodies[i]->velocity + timestep * acceleration;
            bodies[i]->position = bodies[i]->position + timestep * bodies[i]->velocity;
        }
        return;
    }

    std::vector<std::thread> threads;
    threads.reserve(num_threads);
    
    int bodies_per_thread = (n + num_threads - 1) / num_threads;
    
    for (int t = 0; t < num_threads; ++t) {
        int start = t * bodies_per_thread;
        int end = std::min(start + bodies_per_thread, n);
        
        threads.emplace_back([this, &forces, timestep, start, end]() {
            for (int i = start; i < end; ++i) {
                Vector acceleration = forces[i] / bodies[i]->mass;
                bodies[i]->velocity = bodies[i]->velocity + timestep * acceleration;
                bodies[i]->position = bodies[i]->position + timestep * bodies[i]->velocity;
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
}