#include "gamelistview.hpp"
#include <iostream>

GameListView::GameListView()
{
    setup_tree_view();

    set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
    set_child(m_tree_view);
    set_has_frame(true);
}

void GameListView::setup_tree_view()
{
    m_list_store = Gtk::ListStore::create(m_columns);
    m_tree_view.set_model(m_list_store);

    m_tree_view.append_column("Game Name", m_columns.m_col_name);
    m_tree_view.append_column("App ID", m_columns.m_col_app_id);
    m_tree_view.append_column("Install Directory", m_columns.m_col_install_dir);

    for (int i = 0; i < 3; ++i)
    {
        auto column = m_tree_view.get_column(i);
        if (column)
        {
            column->set_resizable(true);
            column->set_sort_column(i);
        }
    }

    auto selection = m_tree_view.get_selection();
    selection->set_mode(Gtk::SelectionMode::SINGLE);
    selection->signal_changed().connect(
        sigc::mem_fun(*this, &GameListView::on_selection_changed));

    m_tree_view.set_enable_search(true);
    m_tree_view.set_search_column(m_columns.m_col_name);
}

void GameListView::set_games(const std::vector<SteamUtils::GameInfo> &games)
{
    m_list_store->clear();

    for (const auto &game : games)
    {
        auto row = *(m_list_store->append());
        row[m_columns.m_col_name] = game.name;
        row[m_columns.m_col_app_id] = game.appId;
        row[m_columns.m_col_install_dir] = game.installDir;
        row[m_columns.m_col_game_ptr] = &game;
    }
}

const SteamUtils::GameInfo *GameListView::get_selected_game() const
{
    auto selection = m_tree_view.get_selection();
    auto iter = selection->get_selected();

    if (iter)
    {
        return (*iter)[m_columns.m_col_game_ptr];
    }

    return nullptr;
}

void GameListView::on_selection_changed()
{
    m_signal_game_selected.emit();
}