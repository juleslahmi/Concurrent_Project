#include "gui.hpp"
#include <cmath>  // For M_PI

// -------- GUI Drawing Area --------

GUI::GUI(Galaxy& galaxy)
    : galaxy(galaxy) {
    set_size_request(1000, 1000);
}

bool GUI::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
    const int w = get_allocated_width();
    const int h = get_allocated_height();

    cr->set_source_rgb(1, 1, 1);  // White background
    cr->paint();
    cr->translate(w / 2, h / 2);  // Center the origin

    const double scale = 1e-7;
    const double radius = 8.0;

    const std::vector<std::tuple<double, double, double>> colors = {
        {0, 0, 1},        // Blue
        {0.5, 0.5, 0.5},  // Gray
        {1, 0, 0},        // Red
        {0, 1, 0},        // Green
        {1, 0.5, 0},      // Orange
        {0.6, 0, 0.6}     // Purple
    };

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

// -------- GUI Window Wrapper --------

GUIWindow::GUIWindow(Galaxy& galaxy)
    : galaxy(galaxy), gui(galaxy) {
    set_title("N-Body Simulation");
    set_default_size(1000, 1000);
    add(gui);
    gui.show();

    // Redraw every 50 ms
    Glib::signal_timeout().connect(sigc::mem_fun(*this, &GUIWindow::on_timeout), 50);
}

bool GUIWindow::on_timeout() {
    double timestep = 3000; // 5 minutes in seconds
    galaxy.simulate(timestep, 1);  // Single-threaded for now

    gui.queue_draw();  // Redraw GUI

    return true;  // Keep timeout active
}
