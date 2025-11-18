#include "logfileview.hpp"
#include <iomanip>
#include <sstream>

LogFileView::LogFileView()
    : Gtk::Box(Gtk::Orientation::VERTICAL), m_header_box(Gtk::Orientation::HORIZONTAL), m_title_label("Log Files"), m_select_all_button("Select All"), m_select_none_button("Select None")
{
    setup_ui();
}

void LogFileView::setup_ui()
{
    set_spacing(5);

    // Header with title and selection buttons
    m_header_box.set_spacing(10);
    m_header_box.set_margin_start(5);
    m_header_box.set_margin_end(5);
    m_header_box.set_margin_top(5);

    m_title_label.set_markup("<b>Log Files</b>");
    m_title_label.set_halign(Gtk::Align::START);
    m_title_label.set_hexpand(true);

    m_select_all_button.signal_clicked().connect(
        sigc::mem_fun(*this, &LogFileView::on_select_all_clicked));
    m_select_none_button.signal_clicked().connect(
        sigc::mem_fun(*this, &LogFileView::on_select_none_clicked));

    m_header_box.append(m_title_label);
    m_header_box.append(m_select_all_button);
    m_header_box.append(m_select_none_button);

    append(m_header_box);

    // Setup tree view
    setup_tree_view();

    m_scrolled_window.set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
    m_scrolled_window.set_child(m_tree_view);
    m_scrolled_window.set_has_frame(true);
    m_scrolled_window.set_vexpand(true);

    append(m_scrolled_window);
}

void LogFileView::setup_tree_view()
{
    m_list_store = Gtk::ListStore::create(m_columns);
    m_tree_view.set_model(m_list_store);

    auto toggle_renderer = Gtk::make_managed<Gtk::CellRendererToggle>();
    toggle_renderer->signal_toggled().connect([this](const Glib::ustring &path)
                                              {
        auto iter = m_list_store->get_iter(path);
        if (iter) {
            bool current = (*iter)[m_columns.m_col_selected];
            (*iter)[m_columns.m_col_selected] = !current;
        } });

    auto toggle_column = Gtk::make_managed<Gtk::TreeViewColumn>("Select", *toggle_renderer);
    toggle_column->add_attribute(toggle_renderer->property_active(), m_columns.m_col_selected);
    m_tree_view.append_column(*toggle_column);

    m_tree_view.append_column("Filename", m_columns.m_col_filename);
    m_tree_view.append_column("Type", m_columns.m_col_type);
    m_tree_view.append_column("Size", m_columns.m_col_size);
    m_tree_view.append_column("Modified", m_columns.m_col_modified);
    m_tree_view.append_column("Path", m_columns.m_col_path);

    for (int i = 1; i < 6; ++i)
    {
        auto column = m_tree_view.get_column(i);
        if (column)
        {
            column->set_resizable(true);
            column->set_sort_column(i);
        }
    }

    auto selection = m_tree_view.get_selection();
    selection->set_mode(Gtk::SelectionMode::MULTIPLE);
}

void LogFileView::set_log_files(const std::vector<SteamUtils::LogFile> &log_files)
{
    m_log_files = log_files;
    m_list_store->clear();

    for (const auto &log_file : log_files)
    {
        auto row = *(m_list_store->append());
        row[m_columns.m_col_selected] = true;
        row[m_columns.m_col_filename] = log_file.filename;
        row[m_columns.m_col_type] = log_file.type;
        row[m_columns.m_col_size] = SteamUtils::formatFileSize(log_file.size);
        row[m_columns.m_col_modified] = log_file.lastModified;
        row[m_columns.m_col_path] = log_file.path;
        row[m_columns.m_col_log_file] = log_file;
    }

    std::ostringstream title;
    title << "<b>Log Files (" << log_files.size() << ")</b>";
    m_title_label.set_markup(title.str());
}

void LogFileView::clear()
{
    m_log_files.clear();
    m_list_store->clear();
    m_title_label.set_markup("<b>Log Files</b>");
}

std::vector<SteamUtils::LogFile> LogFileView::get_selected_log_files() const
{
    std::vector<SteamUtils::LogFile> selected_files;

    for (const auto &row : m_list_store->children())
    {
        if (row[m_columns.m_col_selected])
        {
            selected_files.push_back(row[m_columns.m_col_log_file]);
        }
    }

    return selected_files;
}

void LogFileView::on_select_all_clicked()
{
    for (auto &row : m_list_store->children())
    {
        row[m_columns.m_col_selected] = true;
    }
}

void LogFileView::on_select_none_clicked()
{
    for (auto &row : m_list_store->children())
    {
        row[m_columns.m_col_selected] = false;
    }
}