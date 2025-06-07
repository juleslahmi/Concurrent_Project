#pragma once
#include <gtkmm.h>
#include "galaxy.hpp"

class GUI : public Gtk::DrawingArea {
public:
    explicit GUI(Galaxy& galaxy);

protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;

private:
    Galaxy& galaxy;
};

class GUIWindow : public Gtk::Window {
public:
    explicit GUIWindow(Galaxy& galaxy, int num_threads);

protected:
    bool on_timeout();

private:
    Galaxy& galaxy;
    GUI gui;
    int num_threads;
};
