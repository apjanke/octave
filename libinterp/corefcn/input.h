/*

Copyright (C) 1993-2018 John W. Eaton

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

// Use the GNU readline library for command line editing and hisory.

#if ! defined (octave_input_h)
#define octave_input_h 1

#include "octave-config.h"

#include <cstdio>

#include <string>

#include "hook-fcn.h"
#include "oct-refcount.h"
#include "oct-time.h"
#include "ovl.h"
#include "pager.h"

// TRUE after a call to completion_matches.
extern bool octave_completion_matches_called;

// TRUE if the plotting system has requested a call to drawnow at
// the next user prompt.
extern OCTINTERP_API bool Vdrawnow_requested;

// TRUE if we are in debugging mode.
extern OCTINTERP_API bool Vdebugging;

// TRUE if we are not executing a command direct from debug> prompt.
extern OCTINTERP_API bool Vtrack_line_num;

extern octave::sys::time Vlast_prompt_time;

class octave_value;

namespace octave
{
  class interpreter;
  class base_lexer;

  class input_system
  {
  public:

    input_system (interpreter& interp);

    void initialize (bool line_editing);

    octave_value PS1 (const octave_value_list& args, int nargout);

    std::string PS1 (void) const { return m_PS1; }

    std::string PS1 (const std::string& s)
    {
      std::string val = m_PS1;
      m_PS1 = s;
      return val;
    }

    void set_PS1 (const std::string& s) { m_PS1 = s; }

    octave_value PS2 (const octave_value_list& args, int nargout);

    std::string PS2 (void) const { return m_PS2; }

    std::string PS2 (const std::string& s)
    {
      std::string val = m_PS2;
      m_PS2 = s;
      return val;
    }

    void set_PS2 (const std::string& s) { m_PS2 = s; }

    std::string last_debugging_command (void) const
    {
      return m_last_debugging_command;
    }

    std::string last_debugging_command (const std::string& s)
    {
      std::string val = m_last_debugging_command;
      m_last_debugging_command = s;
      return val;
    }

    octave_value
    completion_append_char (const octave_value_list& args, int nargout);

    char completion_append_char (void) const
    {
      return m_completion_append_char;
    }

    char completion_append_char (char c)
    {
      char val = m_completion_append_char;
      m_completion_append_char = c;
      return val;
    }

    void set_completion_append_char (char c) { m_completion_append_char = c; }

    octave_value gud_mode (const octave_value_list& args, int nargout);

    bool gud_mode (void) const { return m_gud_mode; }

    bool gud_mode (bool flag)
    {
      bool val = m_gud_mode;
      m_gud_mode = flag;
      return val;
    }

    void set_gud_mode (bool flag) { m_gud_mode = flag; }

    octave_value mfile_encoding (const octave_value_list& args, int nargout);

    std::string mfile_encoding (void) const { return m_mfile_encoding; }

    std::string mfile_encoding (const std::string& s)
    {
      std::string val = m_mfile_encoding;
      m_mfile_encoding = s;
      return val;
    }

    void set_mfile_encoding (const std::string& s) { m_mfile_encoding = s; }

    bool yes_or_no (const std::string& prompt);

    std::string interactive_input (const std::string& s, bool& eof);

    octave_value_list
    get_user_input (const octave_value_list& args, int nargout);

    octave_value
    keyboard (const octave_value_list& args = octave_value_list ());

    bool have_input_event_hooks (void) const;

    void add_input_event_hook (const hook_function& hook_fcn);

    bool remove_input_event_hook (const std::string& hook_fcn_id);

    void clear_input_event_hooks (void);

    void run_input_event_hooks (void);

  private:

    interpreter& m_interpreter;

    // Primary prompt string.
    std::string m_PS1;

    // Secondary prompt string.
    std::string m_PS2;

    // Character to append after successful command-line completion
    // attempts.
    char m_completion_append_char;

    // TRUE if we are running in the Emacs GUD mode.
    bool m_gud_mode;

    // Codepage which is used to read .m files
    std::string m_mfile_encoding;

    // If we are in debugging mode, this is the last command entered,
    // so that we can repeat the previous command if the user just
    // types RET.
    std::string m_last_debugging_command;

    hook_function_list m_input_event_hook_functions;

    std::string gnu_readline (const std::string& s, bool& eof) const;

    void get_debug_input (const std::string& prompt);
  };

  class base_reader
  {
  public:

    friend class input_reader;

    base_reader (base_lexer *lxr)
      : m_count (1), m_pflag (0), m_lexer (lxr)
    { }

    base_reader (const base_reader& x)
      : m_count (1), m_pflag (x.m_pflag), m_lexer (x.m_lexer)
    { }

    virtual ~base_reader (void) = default;

    virtual std::string get_input (bool& eof) = 0;

    virtual std::string input_source (void) const { return s_in_src; }

    void reset (void) { promptflag (1); }

    void increment_promptflag (void) { m_pflag++; }

    void decrement_promptflag (void) { m_pflag--; }

    int promptflag (void) const { return m_pflag; }

    int promptflag (int n)
    {
      int retval = m_pflag;
      m_pflag = n;
      return retval;
    }

    std::string octave_gets (bool& eof);

    virtual bool reading_fcn_file (void) const;

    virtual bool reading_classdef_file (void) const;

    virtual bool reading_script_file (void) const;

    virtual bool input_from_terminal (void) const { return false; }

    virtual bool input_from_file (void) const { return false; }

    virtual bool input_from_eval_string (void) const { return false; }

  private:

    refcount<int> m_count;

    int m_pflag;

    base_lexer *m_lexer;

    static const std::string s_in_src;
  };

  class input_reader
  {
  public:

    input_reader (base_lexer *lxr = nullptr);

    input_reader (FILE *file, base_lexer *lxr = nullptr);

    input_reader (const std::string& str, base_lexer *lxr = nullptr);

    input_reader (const input_reader& ir)
    {
      m_rep = ir.m_rep;
      m_rep->m_count++;
    }

    input_reader& operator = (const input_reader& ir)
    {
      if (&ir != this)
        {
          m_rep = ir.m_rep;
          m_rep->m_count++;
        }

      return *this;
    }

    ~input_reader (void)
    {
      if (--m_rep->m_count == 0)
        delete m_rep;
    }

    void reset (void) { return m_rep->reset (); }

    void increment_promptflag (void) { m_rep->increment_promptflag (); }

    void decrement_promptflag (void) { m_rep->decrement_promptflag (); }

    int promptflag (void) const { return m_rep->promptflag (); }

    int promptflag (int n) { return m_rep->promptflag (n); }

    std::string get_input (bool& eof)
    {
      return m_rep->get_input (eof);
    }

    std::string input_source (void) const
    {
      return m_rep->input_source ();
    }

    bool input_from_terminal (void) const
    {
      return m_rep->input_from_terminal ();
    }

    bool input_from_file (void) const
    {
      return m_rep->input_from_file ();
    }

    bool input_from_eval_string (void) const
    {
      return m_rep->input_from_eval_string ();
    }

  private:

    base_reader *m_rep;
  };
}

#if defined (OCTAVE_USE_DEPRECATED_FUNCTIONS)

OCTAVE_DEPRECATED (5, "use 'octave::input_system::set_default_prompts' instead")
extern void set_default_prompts (void);

OCTAVE_DEPRECATED (5, "use 'octave::input_system::yes_or_no' instead")
extern bool octave_yes_or_no (const std::string& prompt);

OCTAVE_DEPRECATED (5, "use 'octave::input_system::keyboard' instead")
extern octave_value do_keyboard (const octave_value_list& args
                                 = octave_value_list ());

OCTAVE_DEPRECATED (5, "use 'octave::input_system::clear_input_event_hooks' instead")
extern void remove_input_event_hook_functions (void);

OCTAVE_DEPRECATED (5, "this function will be removed in a future version of Octave")
extern OCTINTERP_API FILE * get_input_from_stdin (void);

#endif

#endif
