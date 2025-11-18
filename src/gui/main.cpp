#include <gtkmm.h>
#include "mainwindow.hpp"

int main(int argc, char *argv[])
{
    auto app = Gtk::Application::create("com.steamlogcollector.app");

    return app->make_window_and_run<MainWindow>(argc, argv);
}