/*

Copyright (C) 2011-2018 Jacob Dawid

This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/

#if ! defined (octave_file_editor_h)
#define octave_file_editor_h 1

#include <QToolBar>
#include <QAction>
#include <QMenuBar>
#include <QStatusBar>
#include <QCloseEvent>
#include <QTabWidget>
#include <QStackedWidget>

#include <QDragEnterEvent>
#include <QDropEvent>

#include <map>

#include "file-editor-interface.h"
#include "file-editor-tab.h"
#include "tab-bar.h"

namespace octave
{
  // subclassed QTabWidget for using custom tabbar

  class file_editor_tab_widget : public QTabWidget
  {
    Q_OBJECT

  public:

    file_editor_tab_widget (QWidget *p);

    ~file_editor_tab_widget (void) = default;

    tab_bar * get_tab_bar (void) const;
  };

  // the class for the file editor

  class file_editor : public file_editor_interface
  {
    Q_OBJECT

  public:

    struct tab_info
    {
      QWidget *fet_ID;
      QString encoding;
    };

    typedef std::map<QString, tab_info>::iterator editor_tab_map_iterator;
    typedef std::map<QString, tab_info>::const_iterator editor_tab_map_const_iterator;

    // struct that allows to sort with respect to the tab index
    struct session_data
    {
      QString index;
      QString file_name;
      QString encoding;

      bool operator < (const session_data& other) const
      {
        return index < other.index;
      }
    };

    file_editor (QWidget *p);

    ~file_editor (void);

    QMenu * get_mru_menu (void) { return m_mru_file_menu; }

    QMenu * debug_menu (void) { return m_debug_menu; }

    QToolBar * toolbar (void) { return m_tool_bar; }

    void insert_global_actions (QList<QAction*>);

    enum shared_actions_idx
    {
      NEW_SCRIPT_ACTION = 0,
      NEW_FUNCTION_ACTION,
      OPEN_ACTION,
      FIND_FILES_ACTION,
      UNDO_ACTION,
      COPY_ACTION,
      PASTE_ACTION,
      SELECTALL_ACTION
    };

    void handle_enter_debug_mode (void);
    void handle_exit_debug_mode (void);

    void check_actions (void);
    void empty_script (bool startup, bool visible);
    void restore_session (QSettings *settings);

  signals:

    void fetab_settings_changed (const QSettings *settings);
    void fetab_change_request (const QWidget *ID);
    void fetab_file_name_query (const QWidget *ID);
    // Save is a ping-pong type of communication
    void fetab_save_file (const QWidget *ID, const QString& fileName,
                          bool remove_on_success);
    // No fetab_open, functionality in editor
    // No fetab_new, functionality in editor
    void fetab_context_help (const QWidget *ID, bool);
    void fetab_context_edit (const QWidget *ID);
    void fetab_check_modified_file (void);
    void fetab_save_file (const QWidget *ID);
    void fetab_save_file_as (const QWidget *ID);
    void fetab_print_file (const QWidget *ID);
    void fetab_run_file (const QWidget *ID);
    void fetab_context_run (const QWidget *ID);
    void fetab_toggle_bookmark (const QWidget *ID);
    void fetab_next_bookmark (const QWidget *ID);
    void fetab_previous_bookmark (const QWidget *ID);
    void fetab_remove_bookmark (const QWidget *ID);
    void fetab_toggle_breakpoint (const QWidget *ID);
    void fetab_next_breakpoint (const QWidget *ID);
    void fetab_previous_breakpoint (const QWidget *ID);
    void fetab_remove_all_breakpoints (const QWidget *ID);
    void fetab_comment_selected_text (const QWidget *ID, bool);
    void fetab_uncomment_selected_text (const QWidget *ID);
    void fetab_indent_selected_text (const QWidget *ID);
    void fetab_unindent_selected_text (const QWidget *ID);
    void fetab_smart_indent_line_or_selected_text (const QWidget *ID);
    void fetab_convert_eol (const QWidget *ID, QsciScintilla::EolMode eol_mode);
    void fetab_find (const QWidget *ID, QList<QAction *>);
    void fetab_find_next (const QWidget *ID);
    void fetab_find_previous (const QWidget *ID);
    void fetab_goto_line (const QWidget *ID, int line = -1);
    void fetab_move_match_brace (const QWidget *ID, bool select);
    void fetab_completion (const QWidget*);
    void fetab_insert_debugger_pointer (const QWidget *ID, int line = -1);
    void fetab_delete_debugger_pointer (const QWidget *ID, int line = -1);
    void fetab_do_breakpoint_marker (bool insert, const QWidget *ID,
                                     int line = -1, const QString& = "");
    void fetab_set_focus (const QWidget *ID);
    void fetab_scintilla_command (const QWidget *ID, unsigned int sci_msg);

    void fetab_zoom_in (const QWidget *ID);
    void fetab_zoom_out (const QWidget *ID);
    void fetab_zoom_normal (const QWidget *ID);

    void fetab_set_directory (const QString& dir);
    void fetab_recover_from_exit (void);

    void request_settings_dialog (const QString&);
    void execute_command_in_terminal_signal (const QString&);
    void request_open_file_external (const QString& file_name, int line);
    void file_loaded_signal (void);

  public slots:

    void focus (void);
    void set_focus (QWidget *fet);
    void enable_menu_shortcuts (bool);
    bool check_closing (void);

    void request_new_file (const QString& commands);
    void request_close_file (bool);
    void request_close_all_files (bool);
    void request_close_other_files (bool);
    void request_mru_open_file (QAction *action);
    void request_print_file (bool);

    void request_redo (bool);
    void request_cut (bool);
    void request_context_help (bool);
    void request_context_doc (bool);
    void request_context_edit (bool);
    void request_save_file (bool);
    void request_save_file_as (bool);
    void request_run_file (bool);
    void request_context_run (bool);
    void request_toggle_bookmark (bool);
    void request_next_bookmark (bool);
    void request_previous_bookmark (bool);
    void request_remove_bookmark (bool);

    void request_move_match_brace (bool);
    void request_sel_match_brace (bool);
    void request_toggle_breakpoint (bool);
    void request_next_breakpoint (bool);
    void request_previous_breakpoint (bool);
    void request_remove_breakpoint (bool);

    void request_delete_start_word (bool);
    void request_delete_end_word (bool);
    void request_delete_start_line (bool);
    void request_delete_end_line (bool);
    void request_delete_line (bool);
    void request_copy_line (bool);
    void request_cut_line (bool);
    void request_duplicate_selection (bool);
    void request_transpose_line (bool);

    void request_comment_selected_text (bool);
    void request_uncomment_selected_text (bool);
    void request_comment_var_selected_text (bool);

    void request_upper_case (bool);
    void request_lower_case (bool);
    void request_indent_selected_text (bool);
    void request_unindent_selected_text (bool);
    void request_smart_indent_line_or_selected_text (void);
    void request_conv_eol_windows (bool);
    void request_conv_eol_unix (bool);
    void request_conv_eol_mac (bool);

    void request_find (bool);
    void request_find_next (bool);
    void request_find_previous (bool);

    void request_goto_line (bool);
    void request_completion (bool);

    void handle_file_name_changed (const QString& fileName,
                                   const QString& toolTip);
    void handle_tab_close_request (int index);
    void handle_tab_remove_request (void);
    void handle_add_filename_to_list (const QString& fileName,
                                      const QString& encoding, QWidget *ID);
    void active_tab_changed (int index);
    void handle_editor_state_changed (bool enableCopy, bool is_octave_file);
    void handle_mru_add_file (const QString& file_name, const QString& encoding);
    void check_conflict_save (const QString& fileName, bool remove_on_success);

    void handle_insert_debugger_pointer_request (const QString& file, int line);
    void handle_delete_debugger_pointer_request (const QString& file, int line);
    void handle_update_breakpoint_marker_request (bool insert,
                                                  const QString& file, int line,
                                                  const QString& cond);

    void handle_edit_file_request (const QString& file);

    void handle_file_remove (const QString&, const QString&);
    void handle_file_renamed (bool load_new = true);

    // Tells the editor to react on changed settings.
    void notice_settings (const QSettings *settings);

    void set_shortcuts (void);

    void handle_visibility (bool visible);

    void update_octave_directory (const QString& dir);

  protected slots:

    void copyClipboard (void);
    void pasteClipboard (void);
    void selectAll (void);
    void do_undo (void);

  private slots:

    void request_open_file (const QString& fileName,
                            const QString& encoding = QString (),
                            int line = -1, bool debug_pointer = false,
                            bool breakpoint_marker = false, bool insert = true,
                            const QString& cond = "");
    void request_preferences (bool);
    void request_styles_preferences (bool);

    void show_line_numbers (bool);
    void show_white_space (bool);
    void show_eol_chars (bool);
    void show_indent_guides (bool);
    void show_long_line (bool);
    void show_toolbar (bool);
    void show_statusbar (bool);
    void show_hscrollbar (bool);
    void zoom_in (bool);
    void zoom_out (bool);
    void zoom_normal (bool);

    void create_context_menu (QMenu *);
    void edit_status_update (bool, bool);

  protected:

    void closeEvent (QCloseEvent *event);
    void dragEnterEvent (QDragEnterEvent *event);
    void dropEvent (QDropEvent *event);

  private:

    bool is_editor_console_tabbed (void);
    void construct (void);
    void add_file_editor_tab (file_editor_tab *f, const QString& fn);
    void mru_menu_update (void);
    bool call_custom_editor (const QString& file_name = QString (), int line = -1);

    void toggle_preference (const QString& preference, bool def);

    void handle_dir_remove (const QString& old_name, const QString& new_name);

    bool editor_tab_has_focus (void);

    QWidget * find_tab_widget (const QString& openFileName);
    QAction * add_action (QMenu *menu, const QString& text,
                          const char *member, QWidget *receiver = nullptr);
    QAction * add_action (QMenu *menu, const QIcon& icon, const QString& text,
                          const char *member, QWidget *receiver = nullptr);

    QMenu * add_menu (QMenuBar *p, QString text);

    std::map<QString, tab_info> m_editor_tab_map;
    QHash<QMenu*, QStringList> m_hash_menu_text;

    QString m_ced;

    QMenuBar *m_menu_bar;
    QToolBar *m_tool_bar;
    QMenu *m_debug_menu;

    QAction *m_new_action;
    QAction *m_new_function_action;
    QAction *m_open_action;

    QAction *m_upper_case_action;
    QAction *m_lower_case_action;
    QAction *m_comment_selection_action;
    QAction *m_comment_var_selection_action;
    QAction *m_uncomment_selection_action;
    QAction *m_indent_selection_action;
    QAction *m_unindent_selection_action;
    QAction *m_smart_indent_line_or_selection_action;
    QAction *m_conv_eol_windows_action;
    QAction *m_conv_eol_unix_action;
    QAction *m_conv_eol_mac_action;

    QAction *m_copy_action;
    QAction *m_cut_action;
    QAction *m_paste_action;
    QAction *m_selectall_action;
    QAction *m_context_help_action;
    QAction *m_context_doc_action;

    QAction *m_show_linenum_action;
    QAction *m_show_whitespace_action;
    QAction *m_show_eol_action;
    QAction *m_show_indguide_action;
    QAction *m_show_longline_action;
    QAction *m_show_toolbar_action;
    QAction *m_show_statusbar_action;
    QAction *m_show_hscrollbar_action;
    QAction *m_zoom_in_action;
    QAction *m_zoom_out_action;
    QAction *m_zoom_normal_action;

    QAction *m_delete_start_word_action;
    QAction *m_delete_end_word_action;
    QAction *m_delete_start_line_action;
    QAction *m_delete_end_line_action;
    QAction *m_delete_line_action;
    QAction *m_copy_line_action;
    QAction *m_cut_line_action;
    QAction *m_duplicate_selection_action;
    QAction *m_transpose_line_action;

    QAction *m_find_action;
    QAction *m_find_next_action;
    QAction *m_find_previous_action;
    QAction *m_find_files_action;
    QAction *m_goto_line_action;
    QAction *m_completion_action;

    QAction *m_move_to_matching_brace;
    QAction *m_sel_to_matching_brace;
    QAction *m_next_bookmark_action;
    QAction *m_previous_bookmark_action;
    QAction *m_toggle_bookmark_action;
    QAction *m_remove_bookmark_action;

    QAction *m_print_action;
    QAction *m_run_action;
    QAction *m_run_selection_action;

    QAction *m_edit_function_action;
    QAction *m_popdown_mru_action;
    QAction *m_save_action;
    QAction *m_save_as_action;
    QAction *m_close_action;
    QAction *m_close_all_action;
    QAction *m_close_others_action;

    QAction *m_redo_action;
    QAction *m_undo_action;

    QAction *m_preferences_action;
    QAction *m_styles_preferences_action;

    QAction *m_switch_left_tab_action;
    QAction *m_switch_right_tab_action;
    QAction *m_move_tab_left_action;
    QAction *m_move_tab_right_action;

    QAction *m_toggle_breakpoint_action;
    QAction *m_next_breakpoint_action;
    QAction *m_previous_breakpoint_action;
    QAction *m_remove_all_breakpoints_action;

    QMenu *m_edit_menu;
    QMenu *m_edit_cmd_menu;
    QMenu *m_edit_fmt_menu;
    QMenu *m_edit_nav_menu;
    QMenu *m_fileMenu;
    QMenu *m_view_editor_menu;

    QList<QAction*> m_fetab_actions;

    file_editor_tab_widget *m_tab_widget;

    int m_marker_breakpoint;

    bool m_closed;
    bool m_no_focus;

    enum { MaxMRUFiles = 10 };
    QMenu *m_mru_file_menu;
    QAction *m_mru_file_actions[MaxMRUFiles];
    QStringList m_mru_files;
    QStringList m_mru_files_encodings;

    // List of temporarily closed files for later reloading.
    // Order: first closed old file
    //        first new location of closed file
    //        encoding to use for reload
    QStringList m_tmp_closed_files;
  };
}

#endif
