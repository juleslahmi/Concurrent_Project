#include <gtkmm.h>
#include <vector>
#include "vector.hpp"
#include "body.hpp"
#include "galaxy.hpp"
#include "gui.hpp"
#include <iostream>
#include <chrono>
#include <random>


int main(int argc, char *argv[]) {
    /*
    std::vector<Body*> bodies = {
        new Body(1.989e30, Vector(0, 0), Vector(0, 0)),                 // Sun
        new Body(3.30e23, Vector(5.79e10, 0), Vector(0, 47400)),        // Mercury
        new Body(4.87e24, Vector(1.082e11, 0), Vector(0, 35000)),       // Venus
        new Body(5.97e24, Vector(1.496e11, 0), Vector(0, 29780)),       // Earth
        new Body(6.42e23, Vector(2.279e11, 0), Vector(0, 24070)),       // Mars
        new Body(1.90e27, Vector(7.785e11, 0), Vector(0, 13070)),       // Jupiter
        new Body(5.68e26, Vector(1.433e12, 0), Vector(0, 9690)),        // Saturn
        new Body(8.68e25, Vector(2.877e12, 0), Vector(0, 6810)),        // Uranus
        new Body(1.02e26, Vector(4.503e12, 0), Vector(0, 5430))         // Neptune
    };
    */
    const int num_bodies = 1000;
    std::vector<Body*> bodies;
    std::mt19937 rng(42); // fixed seed for reproducibility
    std::uniform_real_distribution<double> pos_dist(-5e12, 5e12);   // random positions
    std::uniform_real_distribution<double> vel_dist(-1e8, 1e8);     // random velocities
    std::uniform_real_distribution<double> mass_dist(1e20, 1e25);   // random masses

    for (int i = 0; i < num_bodies; ++i) {
        double mass = mass_dist(rng);
        Vector pos(pos_dist(rng), pos_dist(rng));
        Vector vel(vel_dist(rng), vel_dist(rng));
        bodies.push_back(new Body(mass, pos, vel));
    }

    bool testing = false;

    if (testing) {
        Galaxy galaxy(bodies);
        const int max_frames = 100;
        const double timestep = 1200;
        const int substeps = 1;
        const int num_threads = 4;

        auto start = std::chrono::high_resolution_clock::now();

        for (int frame = 0; frame < max_frames; ++frame) {
            for (int i = 0; i < substeps; ++i) {
                galaxy.simulate(timestep, num_threads);
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;

        std::cout << "Simulated " << max_frames << " frames ("
                << max_frames * substeps << " steps) with " << num_threads << " thread(s), in "
                << duration.count() << " seconds.\n";

        for (auto& body : bodies) delete body;
        return 0;
    }

    Galaxy galaxy(bodies);
    auto app = Gtk::Application::create(argc, argv, "org.example.sim");
    GUIWindow window(galaxy);
    int result = app->run(window);

    for (auto& body : bodies) delete body;
    return result;
}
