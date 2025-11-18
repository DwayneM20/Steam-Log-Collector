#include "mainwindow.hpp"
#include "gamelistview.hpp"
#include "logfileview.hpp"
#include "logger.hpp"
#include <iostream>
#include <filesystem>

MainWindow::MainWindow()
    : m_main_box(Gtk::Orientation::VERTICAL), m_main_paned(Gtk::Orientation::HORIZONTAL), m_status_box(Gtk::Orientation::HORIZONTAL), m_scan_button("🔍 Scan Games"), m_find_logs_button("📁 Find Logs"), m_copy_logs_button("📋 Copy Logs"), m_menu_button() // No parameter
      ,
      m_status_label("Ready"), m_collector(std::make_unique<SteamLogCollector>())
{
    set_title("Steam Log Collector");
    set_default_size(1000, 700);

    setup_ui();
}

void MainWindow::setup_ui()
{
    setup_header_bar();
    setup_main_content();
    setup_status_bar();
    setup_menu();

    set_child(m_main_box);

    set_buttons_sensitive(true, false, false);
    show_progress(false);
}

void MainWindow::setup_header_bar()
{
    set_titlebar(m_header_bar);
    m_header_bar.set_show_title_buttons(true);

    m_scan_button.add_css_class("suggested-action");
    m_find_logs_button.add_css_class("accent");
    m_copy_logs_button.add_css_class("destructive-action");

    m_menu_button.set_label("☰");

    m_scan_button.signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::on_scan_games_clicked));
    m_find_logs_button.signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::on_find_logs_clicked));
    m_copy_logs_button.signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::on_copy_logs_clicked));

    m_scan_button.set_tooltip_text("Scan for installed Steam games");
    m_find_logs_button.set_tooltip_text("Find log files for selected game");
    m_copy_logs_button.set_tooltip_text("Copy selected log files");

    m_header_bar.pack_start(m_scan_button);
    m_header_bar.pack_start(m_find_logs_button);
    m_header_bar.pack_start(m_copy_logs_button);
    m_header_bar.pack_end(m_menu_button);
}

void MainWindow::setup_main_content()
{
    m_game_list = std::make_unique<GameListView>();
    m_log_view = std::make_unique<LogFileView>();

    m_game_list->signal_game_selected().connect(
        sigc::mem_fun(*this, &MainWindow::on_game_selected));

    m_main_paned.set_start_child(*m_game_list);
    m_main_paned.set_end_child(*m_log_view);
    m_main_paned.set_position(350);
    m_main_paned.set_wide_handle(true);

    m_main_box.append(m_main_paned);
}

void MainWindow::setup_status_bar()
{
    m_status_box.set_spacing(12);
    m_status_box.set_margin_top(8);
    m_status_box.set_margin_bottom(8);
    m_status_box.set_margin_start(12);
    m_status_box.set_margin_end(12);

    m_status_label.set_halign(Gtk::Align::START);
    m_status_label.set_hexpand(true);
    m_status_label.set_ellipsize(Pango::EllipsizeMode::END);

    m_progress_bar.set_show_text(true);
    m_progress_bar.set_visible(false);
    m_progress_bar.set_size_request(200, -1);

    m_status_box.append(m_status_label);
    m_status_box.append(m_progress_bar);

    m_main_box.append(m_status_box);
}

void MainWindow::setup_menu()
{
    m_menu_model = Gio::Menu::create();

    m_menu_model->append("Choose Steam Directory...", "win.choose-steam-dir");
    m_menu_model->append("About", "win.about");

    m_popover_menu.set_menu_model(m_menu_model);
    m_menu_button.set_popover(m_popover_menu);
    m_menu_button.set_direction(Gtk::ArrowType::DOWN);

    add_action("choose-steam-dir", sigc::mem_fun(*this, &MainWindow::on_choose_steam_dir_clicked));
    add_action("about", sigc::mem_fun(*this, &MainWindow::on_about_clicked));
}

void MainWindow::dispatch_to_main_thread(std::function<void()> func)
{
    auto dispatcher = Glib::signal_idle().connect([func]()
                                                  {
                                                      func();
                                                      return false; // Disconnect after one call
                                                  });
}

void MainWindow::on_scan_games_clicked()
{
    if (m_operation_in_progress)
        return;

    m_operation_in_progress = true;
    update_status("Initializing scan...");
    show_progress(true);
    set_buttons_sensitive(false, false, false);

    m_games.clear();
    m_game_list->set_games(m_games);
    m_log_view->clear();
    m_selected_game = nullptr;

    m_collector->scanForGamesAsync(
        [this](double progress, const std::string &message)
        {
            dispatch_to_main_thread([this, progress, message]()
                                    { update_progress(progress, message); });
        },
        [this](std::vector<SteamUtils::GameInfo> games)
        {
            dispatch_to_main_thread([this, games]()
                                    {
                m_games = games;
                m_game_list->set_games(m_games);
                m_steam_directory = m_collector->getSteamDirectory();
                
                std::string status_msg = std::to_string(m_games.size()) + " games found";
                if (!m_steam_directory.empty()) {
                    status_msg += " in " + std::filesystem::path(m_steam_directory).filename().string();
                }
                
                update_status(status_msg);
                show_progress(false);
                set_buttons_sensitive(true, !m_games.empty(), false);
                m_operation_in_progress = false; });
        });
}

void MainWindow::on_find_logs_clicked()
{
    if (!m_selected_game || m_operation_in_progress)
    {
        update_status("Please select a game first");
        return;
    }

    m_operation_in_progress = true;
    update_status("Searching for log files...");
    show_progress(true);
    set_buttons_sensitive(false, false, false);

    m_log_files.clear();
    m_log_view->clear();

    m_collector->findLogsForGameAsync(
        *m_selected_game,
        [this](double progress, const std::string &message)
        {
            dispatch_to_main_thread([this, progress, message]()
                                    { update_progress(progress, message); });
        },
        [this](std::vector<SteamUtils::LogFile> log_files)
        {
            dispatch_to_main_thread([this, log_files]()
                                    {
                m_log_files = log_files;
                m_log_view->set_log_files(m_log_files);
                
                std::string status_msg = std::to_string(m_log_files.size()) + 
                    " log files found for " + m_selected_game->name;
                
                update_status(status_msg);
                show_progress(false);
                set_buttons_sensitive(true, true, !m_log_files.empty());
                m_operation_in_progress = false; });
        });
}

void MainWindow::on_copy_logs_clicked()
{
    if (m_log_files.empty() || !m_selected_game || m_operation_in_progress)
    {
        return;
    }

    auto selected_logs = m_log_view->get_selected_log_files();
    if (selected_logs.empty())
    {
        update_status("No log files selected for copying");
        return;
    }

    m_operation_in_progress = true;
    update_status("Preparing to copy files...");
    show_progress(true);
    set_buttons_sensitive(false, false, false);

    m_collector->copyLogsAsync(
        selected_logs,
        m_selected_game->name,
        [this](double progress, const std::string &message)
        {
            dispatch_to_main_thread([this, progress, message]()
                                    { update_progress(progress, message); });
        },
        [this, selected_logs](int copied_count, const std::string &output_dir)
        {
            dispatch_to_main_thread([this, copied_count, output_dir, selected_logs]()
                                    {
                std::string status_msg;
                if (copied_count > 0) {
                    status_msg = "Successfully copied " + std::to_string(copied_count) + 
                        "/" + std::to_string(selected_logs.size()) + " files to " + 
                        std::filesystem::path(output_dir).filename().string();
                } else {
                    status_msg = "Failed to copy log files";
                }
                
                update_status(status_msg);
                show_progress(false);
                set_buttons_sensitive(true, true, true);
                m_operation_in_progress = false;
                
                if (copied_count > 0) {
                    auto dialog = Gtk::AlertDialog::create("Copy Complete");
                    dialog->set_detail("Files copied to:\n" + output_dir);
                    dialog->set_modal(true);
                    dialog->show(*this);
                } });
        });
}

void MainWindow::on_game_selected()
{
    if (m_operation_in_progress)
        return;

    m_selected_game = m_game_list->get_selected_game();

    if (m_selected_game)
    {
        m_log_files.clear();
        m_log_view->clear();
        update_status("Selected: " + m_selected_game->name + " (ID: " + m_selected_game->appId + ")");
        set_buttons_sensitive(true, true, false);
    }
    else
    {
        update_status("No game selected");
        set_buttons_sensitive(true, false, false);
    }
}

void MainWindow::on_choose_steam_dir_clicked()
{
    if (m_operation_in_progress)
        return;

    auto dialog = Gtk::FileDialog::create();
    dialog->set_title("Select Steam Directory");
    dialog->set_modal(true);

    dialog->select_folder(*this, [this, dialog](const Glib::RefPtr<Gio::AsyncResult> &result)
                          {
        try {
            auto file = dialog->select_folder_finish(result);
            if (file) {
                std::string path = file->get_path();
                if (SteamUtils::isValidSteamDirectory(path)) {
                    m_steam_directory = path;
                    update_status("Steam directory set to: " + std::filesystem::path(path).filename().string());
                    
                    m_games.clear();
                    m_game_list->set_games(m_games);
                    m_log_view->clear();
                    m_selected_game = nullptr;
                    set_buttons_sensitive(true, false, false);
                } else {
                    auto error_dialog = Gtk::AlertDialog::create("Invalid Steam Directory");
                    error_dialog->set_detail("The selected directory is not a valid Steam installation.");
                    error_dialog->set_modal(true);
                    error_dialog->show(*this);
                }
            }
        } catch (const Glib::Error& e) {
            Logger::log("File dialog cancelled or error: " + std::string(e.what()));
        } });
}

void MainWindow::on_about_clicked()
{
    auto about = std::make_unique<Gtk::AboutDialog>();
    about->set_transient_for(*this);
    about->set_modal(true);

    about->set_program_name("Steam Log Collector");
    about->set_version("1.0.0");
    about->set_comments("A tool to find and collect log files from Steam games");
    about->set_website("https://github.com/DwayneM20/steam-log-collector");
    about->set_website_label("GitHub Repository");

    std::vector<Glib::ustring> authors = {"Dwayne M <dmm62620@gmail.com>"};
    about->set_authors(authors);

    about->set_license_type(Gtk::License::MIT_X11);

    about->present();
}

void MainWindow::update_status(const std::string &message)
{
    m_status_label.set_text(message);
    Logger::log(message);
}

void MainWindow::update_progress(double fraction, const std::string &text)
{
    m_progress_bar.set_fraction(fraction);
    if (!text.empty())
    {
        m_progress_bar.set_text(text);
    }
}

void MainWindow::show_progress(bool visible)
{
    m_progress_bar.set_visible(visible);
    if (!visible)
    {
        m_progress_bar.set_fraction(0.0);
        m_progress_bar.set_text("");
    }
}

void MainWindow::set_buttons_sensitive(bool scan, bool find, bool copy)
{
    m_scan_button.set_sensitive(scan);
    m_find_logs_button.set_sensitive(find);
    m_copy_logs_button.set_sensitive(copy);
}