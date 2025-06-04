#include<iostream>
#include <algorithm>

#include <string>
#include <stdio.h>
#include <thread>
#include <vector>
#include <cmath>
#include <gtkmm.h>


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
        Body(const double& mass, const Vector& position, const Vector& velocity) : mass(mass), position(position), velocity(velocity) {}
        double mass;
        Vector position;
        Vector velocity;
};

std::pair<int, int> indexToPair(int k, int n) {
    int i = 0;
    while (k >= n - i - 1) {
        k -= (n - i - 1);
        i++;
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

class Galaxy{
    public:
        static constexpr double G = 6.67430e-11;
        std::vector<Body*> bodies;
        Galaxy(std::vector<Body*>& bodies) : bodies(bodies) {}

        void simulate(double timestep, int num_threads) {
            int n = bodies.size();
            int nb_pairs = n * (n - 1) / 2;
            int block_size = nb_pairs / num_threads;

            std::vector<Vector> pair_forces(nb_pairs, Vector(0, 0));
            std::vector<std::thread> threads(num_threads);

            for (int i = 0; i < num_threads; ++i) {
                int start = i * block_size;
                int end = (i == num_threads - 1) ? nb_pairs : start + block_size;
                threads[i] = std::thread(
                    ConcurrentComputeForces, start, end, std::cref(bodies), std::ref(pair_forces));
            }

            for (auto& t : threads) {
                if (t.joinable()) t.join();
            }

            std::vector<Vector> forces(n, Vector(0, 0));
            for (int k = 0; k < nb_pairs; ++k) {
                auto [i, j] = indexToPair(k, n);
                Vector f = pair_forces[k];
                forces[i] = forces[i] + f;
                forces[j] = forces[j] - f;
            }

            for (size_t i = 0; i < bodies.size(); ++i) {
                Vector acceleration = forces[i] / bodies[i]->mass;
                bodies[i]->velocity = bodies[i]->velocity + timestep * acceleration;
                bodies[i]->position = bodies[i]->position + timestep * bodies[i]->velocity;
            }
        }

};

class GUI : public Gtk::DrawingArea {
    public:
        GUI(Galaxy& galaxy)
        : galaxy(galaxy) {
            set_size_request(1000, 1000);
        }

    protected:
        bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override {
            const int w = get_allocated_width();
            const int h = get_allocated_height();

            cr->set_source_rgb(1, 1, 1);
            cr->paint();
            cr->translate(w / 2, h / 2);

            const double scale = 1e-7;

            const std::vector<std::tuple<double, double, double>> colors = {
                {0, 0, 1},        // Blue
                {0.5, 0.5, 0.5},  // Gray
                {1, 0, 0},        // Red
                {0, 1, 0},        // Green
                {1, 0.5, 0},      // Orange
                {0.6, 0, 0.6}    // Purple
            };

            const double radius = 8.0; 

            for (size_t i = 0; i < galaxy.bodies.size(); ++i) {
                Body* body = galaxy.bodies[i];
                auto [r, g, b] = colors[i % colors.size()];
                cr->set_source_rgb(r, g, b);

                double x = body->position[0] * scale;
                double y = body->position[1] * scale;
                cr->arc(x, y, radius, 0, 2 * M_PI);
                cr->fill();
            }

            return true;
        }

    private:
        Galaxy& galaxy;
};

class GUIWindow : public Gtk::Window {
    public:
        GUIWindow(Galaxy& galaxy)
        : galaxy(galaxy), gui(galaxy) {
            set_title("N-Body Simulation");
            set_default_size(1000, 1000);
            add(gui);
            gui.show();
            Glib::signal_timeout().connect(sigc::mem_fun(*this, &GUIWindow::on_timeout), 1);
        }

    protected:
        bool on_timeout() {
            double timestep = 1000; //5mins
            galaxy.simulate(timestep, 1);

            gui.queue_draw();

            return true;
        }

    private:
        Galaxy& galaxy;
        GUI gui;
};

int main(int argc, char *argv[]) {
    std::vector<Body*> bodies = {
        new Body(5e24, Vector(0, 0), Vector(0, -3000)),     
        new Body(7e22, Vector(3.8e8, 0), Vector(0, -2022)),
        new Body(2e27, Vector(-2e9, -1e9), Vector(0, 0))
    };

    Galaxy galaxy(bodies);
    auto app = Gtk::Application::create(argc, argv, "org.example.sim");
    GUIWindow window(galaxy);
    int result = app->run(window);

    for (auto& body : bodies) delete body;
    return result;
}
