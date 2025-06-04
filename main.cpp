#include <gtkmm.h>
#include <vector>
#include "vector.hpp"
#include "body.hpp"
#include "galaxy.hpp"
#include "gui.hpp"

int main(int argc, char *argv[]) {
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

    Galaxy galaxy(bodies);
    auto app = Gtk::Application::create(argc, argv, "org.example.sim");
    GUIWindow window(galaxy);
    int result = app->run(window);

    for (auto& body : bodies) delete body;
    return result;
}
