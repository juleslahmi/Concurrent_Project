#include <gtkmm.h>
#include <vector>
#include "vector.hpp"
#include "body.hpp"
#include "galaxy.hpp"
#include "gui.hpp"

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
