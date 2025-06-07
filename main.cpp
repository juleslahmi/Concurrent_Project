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
    int num_bodies = 100;
    int num_threads = 1;
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
}

int main(int argc, char *argv[]) {
    Config config = parse_args(argc, argv);
    std::vector<Body*> bodies = generate_bodies(config.num_bodies);
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
    if (config.testing) {
        Galaxy galaxy(bodies);
        const int max_frames = 1000;
        const double timestep = 300;
        const int substeps = 1;

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

    Galaxy galaxy(bodies);

    int gtk_argc = 1;
    char* gtk_args[] = { argv[0], nullptr };
    char** gtk_argv = gtk_args;

    auto app = Gtk::Application::create(gtk_argc, gtk_argv, "org.example.sim");
    GUIWindow window(galaxy);
    window.show();
    int result = app->run(window);

    for (auto& body : bodies) delete body;
    return result;
}
