/*

Copyright (C) 2013-2018 John W. Eaton
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

#if ! defined (octave_welcome_wizard_h)
#define octave_welcome_wizard_h 1

#include <QDialog>
#include <QCheckBox>
#include <QLabel>

namespace octave
{
  class welcome_wizard : public QDialog
  {
    Q_OBJECT

  public:

    typedef QWidget *(*page_creator_fptr) (welcome_wizard *wizard);

    welcome_wizard (QWidget *parent = nullptr);

    ~welcome_wizard (void) = default;

  private:

    QList<page_creator_fptr> m_page_ctor_list;
    QList<page_creator_fptr>::iterator m_page_list_iterator;
    QWidget *m_current_page;
    bool m_allow_web_connect_state;

  private slots:

    void handle_web_connect_option (int state);

    void show_page (void);
    void previous_page (void);
    void next_page (void);

    void accept (void);
  };


  class initial_page : public QWidget
  {
    Q_OBJECT

  public:

    initial_page (welcome_wizard *wizard);

    ~initial_page (void) = default;

    static QWidget *
    create (welcome_wizard *wizard) { return new initial_page (wizard); }

  private:

    QLabel *m_title;
    QLabel *m_message;
    QLabel *m_logo;
    QPushButton *m_next;
    QPushButton *m_cancel;
  };


  class setup_community_news : public QWidget
  {
    Q_OBJECT

  public:

    setup_community_news (welcome_wizard *wizard);

    ~setup_community_news (void) = default;

    static QWidget *
    create (welcome_wizard *wizard) { return new setup_community_news (wizard); }

  private:

    QLabel *m_title;
    QLabel *m_message;
    QCheckBox *m_checkbox;
    QLabel *m_checkbox_message;
    QLabel *m_logo;
    QPushButton *m_previous;
    QPushButton *m_next;
    QPushButton *m_cancel;
  };


  class final_page : public QWidget
  {
    Q_OBJECT

  public:

    final_page (welcome_wizard *wizard);

    ~final_page (void) = default;

    static QWidget *
    create (welcome_wizard *wizard) { return new final_page (wizard); }

  private:

    QLabel *m_title;
    QLabel *m_message;
    QLabel *m_logo;
    QLabel *m_links;
    QPushButton *m_previous;
    QPushButton *m_finish;
    QPushButton *m_cancel;
  };
}

#endif
