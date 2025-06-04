#include <gtkmm.h>
#include <vector>
#include "vector.hpp"
#include "body.hpp"
#include "galaxy.hpp"
#include "gui.hpp"

int main(int argc, char *argv[]) {
    std::vector<Body*> bodies = {
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


    Galaxy galaxy(bodies);
    auto app = Gtk::Application::create(argc, argv, "org.example.sim");
    GUIWindow window(galaxy);
    int result = app->run(window);

    for (auto& body : bodies) delete body;
    return result;
}
