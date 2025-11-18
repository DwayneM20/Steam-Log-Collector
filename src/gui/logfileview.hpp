#ifndef LOGFILEVIEW_HPP
#define LOGFILEVIEW_HPP

#include <gtkmm.h>
#include "steam-utils.hpp"

class LogFileView : public Gtk::Box
{
public:
    LogFileView();
    virtual ~LogFileView() = default;

    void set_log_files(const std::vector<SteamUtils::LogFile> &log_files);
    void clear();

    std::vector<SteamUtils::LogFile> get_selected_log_files() const;

private:
    void setup_ui();
    void setup_tree_view();
    void on_select_all_clicked();
    void on_select_none_clicked();

    Gtk::Box m_header_box;
    Gtk::Label m_title_label;
    Gtk::Button m_select_all_button;
    Gtk::Button m_select_none_button;

    Gtk::ScrolledWindow m_scrolled_window;
    Gtk::TreeView m_tree_view;
    Glib::RefPtr<Gtk::ListStore> m_list_store;

    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        ModelColumns()
        {
            add(m_col_selected);
            add(m_col_filename);
            add(m_col_type);
            add(m_col_size);
            add(m_col_modified);
            add(m_col_path);
            add(m_col_log_file);
        }

        Gtk::TreeModelColumn<bool> m_col_selected;
        Gtk::TreeModelColumn<Glib::ustring> m_col_filename;
        Gtk::TreeModelColumn<Glib::ustring> m_col_type;
        Gtk::TreeModelColumn<Glib::ustring> m_col_size;
        Gtk::TreeModelColumn<Glib::ustring> m_col_modified;
        Gtk::TreeModelColumn<Glib::ustring> m_col_path;
        Gtk::TreeModelColumn<SteamUtils::LogFile> m_col_log_file;
    };

    ModelColumns m_columns;
    std::vector<SteamUtils::LogFile> m_log_files;
};

#endif