#include <gtkmm.h>
#include <vector>
#include "vector.hpp"
#include "body.hpp"
#include "galaxy.hpp"
#include "gui.hpp"
#include <iostream>
#include <chrono>
#include <random>


struct Config {
    bool testing = false;
    int num_bodies = 1000;
    int num_threads = 10;
    int version = 1;  // Add version parameter (default: 0)
};

Config parse_args(int argc, char* argv[]) {
    Config config;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--testing" || arg == "-t") {
            config.testing = true;
        } else if (arg == "--bodies" && i + 1 < argc) {
            config.num_bodies = std::stoi(argv[++i]);
        } else if (arg == "--threads" && i + 1 < argc) {
            config.num_threads = std::stoi(argv[++i]);
        } else if (arg == "--version" && i + 1 < argc) {
            config.version = std::stoi(argv[++i]);
        } else if (arg == "1") {
            config.testing = true;
        } else if (arg == "0") {
            config.testing = false;
        }
    }

    return config;
}

// ------------ Body Generation ------------
std::vector<Body*> generate_bodies(int num_bodies) {
    std::vector<Body*> bodies;
    std::mt19937 rng(42); // fixed seed
    std::uniform_real_distribution<double> pos_dist(-5e12, 5e12);
    std::uniform_real_distribution<double> vel_dist(-1e8, 1e8);
    std::uniform_real_distribution<double> mass_dist(1e20, 1e25);

    for (int i = 0; i < num_bodies; ++i) {
        double mass = mass_dist(rng);
        Vector pos(pos_dist(rng), pos_dist(rng));
        Vector vel(vel_dist(rng), vel_dist(rng));
        bodies.push_back(new Body(mass, pos, vel));
    }

    return bodies;

    /*

    Solar System :

    std::vector<Body*> bodies = {}
    new Body(1.989e30, Vector(0, 0), Vector(0, 0), 4.0, {1.0, 0.9, 0.0}), //sun
    new Body(3.30e23, Vector(5.79e10, 0), Vector(0, 47400), 1.0, {0.6, 0.6, 0.6}), //mercury
    new Body(4.87e24, Vector(1.082e11, 0), Vector(0, 35000), 1.4, {0.9, 0.8, 0.5}), //venus
    new Body(5.97e24, Vector(1.496e11, 0), Vector(0, 29780), 3, {0.2, 0.4, 1.0}), //earth
    new Body(6.42e23, Vector(2.279e11, 0), Vector(0, 24070), 2.5, {0.8, 0.3, 0.1}), //mars
    new Body(1.90e27, Vector(7.785e11, 0), Vector(0, 13070), 12.0, {0.9, 0.7, 0.4}), //jupiter
    new Body(5.68e26, Vector(1.433e12, 0), Vector(0, 9690), 11.0, {0.9, 0.85, 0.6}), //saturn
    new Body(8.68e25, Vector(2.877e12, 0), Vector(0, 6810), 9.0, {0.5, 0.9, 1.0}), //uranus
    new Body(1.02e26, Vector(4.503e12, 0), Vector(0, 5430), 9.0, {0.3, 0.4, 1.0}) //neptune
};
    return bodies
    */
}

int main(int argc, char *argv[]) {
    Config config = parse_args(argc, argv);
    std::vector<Body*> bodies = generate_bodies(config.num_bodies);
    
    if (config.testing) {
        Galaxy galaxy(bodies, config.version);
        
        const int max_frames = 1000;
        const double timestep = 300;
        const int substeps = 1;

        std::cout << "Using simulation algorithm version " << config.version << std::endl;
        
        auto start = std::chrono::high_resolution_clock::now();

        for (int frame = 0; frame < max_frames; ++frame) {
            for (int i = 0; i < substeps; ++i) {
                galaxy.simulate(timestep, config.num_threads);
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;

        std::cout << "Simulated " << max_frames << " frames ("
                  << max_frames * substeps << " steps) with "
                  << config.num_threads << " thread(s), in "
                  << duration.count() << " seconds.\n";

        for (auto& body : bodies) delete body;
        return 0;
    }

    // For non-testing mode, use version 0 by default
    Galaxy galaxy(bodies, config.version);

    int gtk_argc = 1;
    char* gtk_args[] = { argv[0], nullptr };
    char** gtk_argv = gtk_args;

    auto app = Gtk::Application::create(gtk_argc, gtk_argv, "org.example.sim");
    GUIWindow window(galaxy, config.num_threads);
    window.show();
    int result = app->run(window);

    for (auto& body : bodies) delete body;
    return result;
}