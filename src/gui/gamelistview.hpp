#ifndef GAMELISTVIEW_HPP
#define GAMELISTVIEW_HPP

#include <gtkmm.h>
#include "steam-utils.hpp"

class GameListView : public Gtk::ScrolledWindow
{
public:
    GameListView();
    virtual ~GameListView() = default;

    void set_games(const std::vector<SteamUtils::GameInfo> &games);
    const SteamUtils::GameInfo *get_selected_game() const;

    sigc::signal<void()> signal_game_selected() { return m_signal_game_selected; }

protected:
    void on_selection_changed();

private:
    void setup_tree_view();

    Gtk::TreeView m_tree_view;
    Glib::RefPtr<Gtk::ListStore> m_list_store;

    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        ModelColumns()
        {
            add(m_col_name);
            add(m_col_app_id);
            add(m_col_install_dir);
            add(m_col_game_ptr);
        }

        Gtk::TreeModelColumn<Glib::ustring> m_col_name;
        Gtk::TreeModelColumn<Glib::ustring> m_col_app_id;
        Gtk::TreeModelColumn<Glib::ustring> m_col_install_dir;
        Gtk::TreeModelColumn<const SteamUtils::GameInfo *> m_col_game_ptr;
    };

    ModelColumns m_columns;
    sigc::signal<void()> m_signal_game_selected;
};

#endif