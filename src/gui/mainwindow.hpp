#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <gtkmm.h>
#include <memory>
#include "steam-utils.hpp"
#include "steam-core.hpp"

class GameListView;
class LogFileView;

class MainWindow : public Gtk::ApplicationWindow
{
public:
    MainWindow();
    virtual ~MainWindow() = default;

protected:
    void on_scan_games_clicked();
    void on_find_logs_clicked();
    void on_copy_logs_clicked();
    void on_game_selected();
    void on_choose_steam_dir_clicked();
    void on_about_clicked();

    void update_status(const std::string &message);
    void update_progress(double fraction, const std::string &text = "");
    void show_progress(bool visible);
    void set_buttons_sensitive(bool scan, bool find, bool copy);

private:
    void setup_ui();
    void setup_header_bar();
    void setup_main_content();
    void setup_status_bar();
    void setup_menu();

    void dispatch_to_main_thread(std::function<void()> func);

    Gtk::HeaderBar m_header_bar;
    Gtk::Button m_scan_button;
    Gtk::Button m_find_logs_button;
    Gtk::Button m_copy_logs_button;
    Gtk::MenuButton m_menu_button;

    Gtk::Box m_main_box;
    Gtk::Paned m_main_paned;

    std::unique_ptr<GameListView> m_game_list;
    std::unique_ptr<LogFileView> m_log_view;

    Gtk::Box m_status_box;
    Gtk::Label m_status_label;
    Gtk::ProgressBar m_progress_bar;

    Glib::RefPtr<Gio::Menu> m_menu_model;
    Gtk::PopoverMenu m_popover_menu;

    std::unique_ptr<SteamLogCollector> m_collector;

    std::vector<SteamUtils::GameInfo> m_games;
    std::vector<SteamUtils::LogFile> m_log_files;
    std::string m_steam_directory;
    const SteamUtils::GameInfo *m_selected_game = nullptr;

    // State management
    bool m_operation_in_progress = false;
};

#endif