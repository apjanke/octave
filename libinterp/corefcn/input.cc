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

// Get command input interactively or from files.

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <iostream>
#include <sstream>
#include <string>

#include "cmd-edit.h"
#include "file-ops.h"
#include "iconv-wrappers.h"
#include "localcharset-wrapper.h"
#include "quit.h"
#include "str-vec.h"
#include "uniconv-wrappers.h"

#include "builtin-defun-decls.h"
#include "call-stack.h"
#include "defun.h"
#include "dirfns.h"
#include "error.h"
#include "errwarn.h"
#include "help.h"
#include "hook-fcn.h"
#include "input.h"
#include "interpreter-private.h"
#include "interpreter.h"
#include "lex.h"
#include "load-path.h"
#include "octave.h"
#include "octave-link.h"
#include "oct-map.h"
#include "oct-hist.h"
#include "interpreter.h"
#include "octave-link.h"
#include "ovl.h"
#include "ov-fcn-handle.h"
#include "ov-usr-fcn.h"
#include "pager.h"
#include "parse.h"
#include "pt-eval.h"
#include "pt-stmt.h"
#include "sighandlers.h"
#include "sysdep.h"
#include "interpreter.h"
#include "unwind-prot.h"
#include "utils.h"
#include "variables.h"

// The time we last printed a prompt.
octave::sys::time Vlast_prompt_time = 0.0;

// TRUE after a call to completion_matches.
bool octave_completion_matches_called = false;

// TRUE if the plotting system has requested a call to drawnow at
// the next user prompt.
bool Vdrawnow_requested = false;

// TRUE if we are in debugging mode.
bool Vdebugging = false;

// TRUE if we are recording line numbers in a source file.
// Always true except when debugging and taking input directly from
// the terminal.
bool Vtrack_line_num = true;

static std::string
quoting_filename (const std::string& text, int, char quote)
{
  if (quote)
    return text;
  else
    return ("'" + text);
}

// Try to parse a partial command line in reverse, excluding trailing TEXT.
// If it appears a variable has been indexed by () or {},
// return that expression,
// to allow autocomplete of field names of arrays of structures.
static std::string
find_indexed_expression (const std::string& text)
{
  std::string line = octave::command_editor::get_line_buffer ();

  int pos = line.length () - text.length ();
  int curly_count = 0;
  int paren_count = 0;

  int last = --pos;

  while (pos >= 0 && (line[pos] == ')' || line[pos] == '}'))
    {
      if (line[pos] == ')')
        paren_count++;
      else if (line[pos] == '}')
        curly_count++;

      while (curly_count + paren_count > 0 && --pos >= 0)
        {
          if (line[pos] == ')')
            paren_count++;
          else if (line[pos] == '(')
            paren_count--;
          else if (line[pos] == '}')
            curly_count++;
          else if (line[pos] == '{')
            curly_count--;
        }

      while (--pos >= 0 && line[pos] == ' ')
        ;
    }

  while (pos >= 0 && (isalnum (line[pos]) || line[pos] == '_'))
    pos--;

  if (++pos >= 0)
    return (line.substr (pos, last + 1 - pos));
  else
    return std::string ();
}

static inline bool
is_variable (octave::symbol_table& symtab, const std::string& name)
{
  bool retval = false;

  if (! name.empty ())
    {
      octave::symbol_scope scope = symtab.current_scope ();

      octave_value val = scope ? scope.varval (name) : octave_value ();

      retval = val.is_defined ();
    }

  return retval;
}

static string_vector
generate_struct_completions (const std::string& text,
                             std::string& prefix, std::string& hint)
{
  string_vector names;

  size_t pos = text.rfind ('.');
  bool array = false;

  if (pos != std::string::npos)
    {
      if (pos == text.length ())
        hint = "";
      else
        hint = text.substr (pos+1);

      prefix = text.substr (0, pos);

      if (prefix == "")
        {
          array = true;
          prefix = find_indexed_expression (text);
        }

      std::string base_name = prefix;

      pos = base_name.find_first_of ("{(. ");

      if (pos != std::string::npos)
        base_name = base_name.substr (0, pos);

      octave::symbol_table& symtab
        = octave::__get_symbol_table__ ("generate_struct_completions");

      if (is_variable (symtab, base_name))
        {
          int parse_status;

          octave::unwind_protect frame;

          frame.protect_var (discard_error_messages);
          frame.protect_var (discard_warning_messages);

          discard_error_messages = true;
          discard_warning_messages = true;

          try
            {
              octave_value tmp = octave::eval_string (prefix, true, parse_status);

              frame.run ();

              if (tmp.is_defined ()
                  && (tmp.isstruct () || tmp.isjava () || tmp.is_classdef_object ()))
                names = tmp.map_keys ();
            }
          catch (const octave::execution_exception&)
            {
              octave::interpreter::recover_from_exception ();
            }
        }
    }

  // Undo look-back that found the array expression,
  // but insert an extra "." to distinguish from the non-struct case.
  if (array)
    prefix = ".";

  return names;
}

// FIXME: this will have to be much smarter to work "correctly".
static bool
looks_like_struct (const std::string& text, char prev_char)
{
  bool retval = (! text.empty ()
                 && (text != "." || prev_char == ')' || prev_char == '}')
                 && text.find_first_of (octave::sys::file_ops::dir_sep_chars ()) == std::string::npos
                 && text.find ("..") == std::string::npos
                 && text.rfind ('.') != std::string::npos);

#if 0
  symbol_record *sr = curr_sym_tab->lookup (text);

  if (sr && ! sr->is_function ())
    {
      int parse_status;

      octave::unwind_protect frame;

      frame.protect_var (discard_error_messages);

      discard_error_messages = true;

      octave_value tmp = eval_string (text, true, parse_status);

      frame.run ();

      retval = (tmp.is_defined () && tmp.isstruct ());
    }
#endif

  return retval;
}

// FIXME: make this generate filenames when appropriate.

static string_vector
generate_possible_completions (const std::string& text, std::string& prefix,
                               std::string& hint, bool& deemed_struct)
{
  string_vector names;

  prefix = "";

  char prev_char = octave::command_editor::get_prev_char (text.length ());
  deemed_struct = looks_like_struct (text, prev_char);

  if (deemed_struct)
    names = generate_struct_completions (text, prefix, hint);
  else
    names = octave::make_name_list ();

  // Sort and remove duplicates.

  names.sort (true);

  return names;
}

static bool
is_completing_dirfns (void)
{
  static std::string dirfns_commands[] = {"cd", "ls"};
  static const size_t dirfns_commands_length = 2;

  bool retval = false;

  std::string line = octave::command_editor::get_line_buffer ();

  for (size_t i = 0; i < dirfns_commands_length; i++)
    {
      int index = line.find (dirfns_commands[i] + ' ');

      if (index == 0)
        {
          retval = true;
          break;
        }
    }

  return retval;
}

static std::string
generate_completion (const std::string& text, int state)
{
  std::string retval;

  static std::string prefix;
  static std::string hint;

  static size_t hint_len = 0;

  static int list_index = 0;
  static int name_list_len = 0;
  static int name_list_total_len = 0;
  static string_vector name_list;
  static string_vector file_name_list;

  static int matches = 0;

  if (state == 0)
    {
      list_index = 0;

      prefix = "";

      hint = text;

      // No reason to display symbols while completing a
      // file/directory operation.

      bool deemed_struct = false;

      if (is_completing_dirfns ())
        name_list = string_vector ();
      else
        name_list = generate_possible_completions (text, prefix, hint,
                                                   deemed_struct);

      name_list_len = name_list.numel ();

      // If the line was something like "a{1}." then text = "." but
      // we don't want to expand all the . files.
      if (! deemed_struct)
        {

          file_name_list = octave::command_editor::generate_filename_completions (text);

          name_list.append (file_name_list);

        }

      name_list_total_len = name_list.numel ();

      hint_len = hint.length ();

      matches = 0;

      for (int i = 0; i < name_list_len; i++)
        if (hint == name_list[i].substr (0, hint_len))
          matches++;
    }

  if (name_list_total_len > 0 && matches > 0)
    {
      while (list_index < name_list_total_len)
        {
          std::string name = name_list[list_index];

          list_index++;

          if (hint == name.substr (0, hint_len))
            {
              // Special case: array reference forces prefix="."
              //               in generate_struct_completions ()
              if (list_index <= name_list_len && ! prefix.empty ())
                retval = (prefix == "." ? "" : prefix) + '.' + name;
              else
                retval = name;

              char prev_char = octave::command_editor::get_prev_char
                                                       (text.length ());
              if (matches == 1 && looks_like_struct (retval, prev_char))
                {
                  // Don't append anything, since we don't know
                  // whether it should be '(' or '.'.

                  octave::command_editor::set_completion_append_character ('\0');
                }
              else
                {
                  octave::input_system& input_sys
                    = octave::__get_input_system__ ("generate_completion");

                  octave::command_editor::set_completion_append_character
                    (input_sys.completion_append_char ());
                }

              break;
            }
        }
    }

  return retval;
}

namespace octave
{
  // Use literal "octave" in default setting for PS1 instead of
  // "\\s" to avoid setting the prompt to "octave.exe" or
  // "octave-gui", etc.

  input_system::input_system (interpreter& interp)
    : m_interpreter (interp), m_PS1 (R"(octave:\#> )"), m_PS2 ("> "),
      m_completion_append_char (' '), m_gud_mode (false),
      m_mfile_encoding ("utf-8"), m_last_debugging_command ("\n"),
      m_input_event_hook_functions ()
  {
#if defined (OCTAVE_USE_WINDOWS_API)
    m_mfile_encoding = "system";
#endif
  }

    void input_system::initialize (bool line_editing)
    {
// Force default line editor if we don't want readline editing.
      if (! line_editing)
        {
          command_editor::force_default_editor ();
          return;
        }

      // If we are using readline, this allows conditional parsing of the
      // .inputrc file.

      octave::command_editor::set_name ("Octave");

      // FIXME: this needs to include a comma too, but that
      // causes trouble for the new struct element completion code.

      static const char *s = "\t\n !\"\'*+-/:;<=>(){}[\\]^`~";

      octave::command_editor::set_basic_word_break_characters (s);

      octave::command_editor::set_completer_word_break_characters (s);

      octave::command_editor::set_basic_quote_characters (R"(")");

      octave::command_editor::set_filename_quote_characters (" \t\n\\\"'@<>=;|&()#$`?*[!:{");

      octave::command_editor::set_completer_quote_characters (R"('")");

      octave::command_editor::set_completion_function (generate_completion);

      octave::command_editor::set_quoting_function (quoting_filename);
  }

  octave_value
  input_system::PS1 (const octave_value_list& args, int nargout)
  {
    return set_internal_variable (m_PS1, args, nargout, "PS1");
  }

  octave_value
  input_system::PS2 (const octave_value_list& args, int nargout)
  {
    return set_internal_variable (m_PS2, args, nargout, "PS2");
  }

  octave_value
  input_system::completion_append_char (const octave_value_list& args,
                                        int nargout)
  {
    return set_internal_variable (m_completion_append_char, args, nargout,
                                  "completion_append_char");
  }

  octave_value
  input_system::gud_mode (const octave_value_list& args, int nargout)
  {
    return set_internal_variable (m_gud_mode, args, nargout, "__gud_mode__");
  }

  octave_value
  input_system::mfile_encoding (const octave_value_list& args, int nargout)
  {
    // Save current value in case there is an error in the additional
    // validation below.

    std::string saved_encoding = m_mfile_encoding;

    // We must pass the actual variable to change here for temporary
    // "local" settings to work properly.

    octave_value retval
      = set_internal_variable (m_mfile_encoding, args, nargout,
                               "__mfile_encoding__");

    // Additional validation if the encoding has changed.

    if (m_mfile_encoding != saved_encoding)
      {
        if (m_mfile_encoding.empty ())
          {
#if defined (OCTAVE_USE_WINDOWS_API)
            m_mfile_encoding = "system";
#else
            m_mfile_encoding = "utf-8";
#endif
          }
        else
          {
            std::transform (m_mfile_encoding.begin (),
                            m_mfile_encoding.end (),
                            m_mfile_encoding.begin (), ::tolower);

            std::string codepage = (m_mfile_encoding.compare ("system") == 0)
              ? octave_locale_charset_wrapper () : m_mfile_encoding;

            // Check for valid codepage.
            void *codec
              = octave_iconv_open_wrapper (codepage.c_str (), "utf-8");

            if (errno == EINVAL)
              {
                m_mfile_encoding = saved_encoding;

                error ("__mfile_encoding__: conversion from codepage '%s' not supported",
                       codepage.c_str ());
              }

            octave_iconv_close_wrapper (codec);
          }
      }

    return retval;
  }

  bool input_system::yes_or_no (const std::string& prompt)
  {
    std::string prompt_string = prompt + "(yes or no) ";

    while (1)
      {
        bool eof = false;

        std::string input_buf = interactive_input (prompt_string, eof);

        if (input_buf == "yes")
          return true;
        else if (input_buf == "no")
          return false;
        else
          message (nullptr, "Please answer yes or no.");
      }
  }

  std::string input_system::interactive_input (const std::string& s, bool& eof)
  {
    Vlast_prompt_time.stamp ();

    if (Vdrawnow_requested && octave::application::interactive ())
      {
        bool eval_error = false;

        try
          {
            Fdrawnow ();
          }
        catch (const octave::execution_exception& e)
          {
            eval_error = true;

            std::string stack_trace = e.info ();

            if (! stack_trace.empty ())
              std::cerr << stack_trace;

            if (octave::application::interactive ())
              octave::interpreter::recover_from_exception ();
          }

        octave::flush_stdout ();

        // We set Vdrawnow_requested to false even if there is an error in
        // drawnow so that the error doesn't reappear at every prompt.

        Vdrawnow_requested = false;

        if (eval_error)
          return "\n";
      }

    return gnu_readline (s, eof);
  }

  // If the user simply hits return, this will produce an empty matrix.

  octave_value_list
  input_system::get_user_input (const octave_value_list& args, int nargout)
  {
    octave_value_list retval;

    int read_as_string = 0;

    if (args.length () == 2)
      read_as_string++;

    std::string prompt = args(0).xstring_value ("input: unrecognized argument");

    output_system& output_sys = __get_output_system__ ("do_sync");

    output_sys.reset ();

    octave_diary << prompt;

    bool eof = false;

    std::string input_buf = interactive_input (prompt.c_str (), eof);

    if (input_buf.empty ())
      error ("input: reading user-input failed!");

    size_t len = input_buf.length ();

    octave_diary << input_buf;

    if (input_buf[len - 1] != '\n')
      octave_diary << "\n";

    if (len < 1)
      return read_as_string ? octave_value ("") : octave_value (Matrix ());

    if (read_as_string)
      {
        // FIXME: fix gnu_readline and octave_gets instead!
        if (input_buf.length () == 1 && input_buf[0] == '\n')
          retval(0) = "";
        else
          retval(0) = input_buf;
      }
    else
      {
        int parse_status = 0;

        retval = octave::eval_string (input_buf, true, parse_status, nargout);

        if (! Vdebugging && retval.empty ())
          retval(0) = Matrix ();
      }

    return retval;
  }

  octave_value input_system::keyboard (const octave_value_list& args)
  {
    octave_value retval;

    int nargin = args.length ();

    assert (nargin == 0 || nargin == 1);

    octave::unwind_protect frame;

    frame.add_fcn (octave::command_history::ignore_entries,
                   octave::command_history::ignoring_entries ());

    octave::command_history::ignore_entries (false);

    frame.protect_var (Vdebugging);

    octave::call_stack& cs = m_interpreter.get_call_stack ();

    frame.add_method (cs, &octave::call_stack::restore_frame,
                      cs.current_frame ());

    // FIXME: probably we just want to print one line, not the
    // entire statement, which might span many lines...
    //
    // tree_print_code tpc (octave_stdout);
    // stmt.accept (tpc);

    Vdebugging = true;
    Vtrack_line_num = false;

    std::string prompt = "debug> ";
    if (nargin > 0)
      prompt = args(0).string_value ();

    get_debug_input (prompt);

    return retval;
  }

  bool input_system::have_input_event_hooks (void) const
  {
    return ! m_input_event_hook_functions.empty ();
  }

  void input_system::add_input_event_hook (const hook_function& hook_fcn)
  {
    m_input_event_hook_functions.insert (hook_fcn.id (), hook_fcn);
  }

  bool input_system::remove_input_event_hook (const std::string& hook_fcn_id)
  {
    hook_function_list::iterator p
      = m_input_event_hook_functions.find (hook_fcn_id);

    if (p == m_input_event_hook_functions.end ())
      return false;

    m_input_event_hook_functions.erase (p);
    return true;
  }

  void input_system::clear_input_event_hooks (void)
  {
    m_input_event_hook_functions.clear ();
  }

  void input_system::run_input_event_hooks (void)
  {
    m_input_event_hook_functions.run ();
  }

  std::string
  input_system::gnu_readline (const std::string& s, bool& eof) const
  {
    octave_quit ();

    eof = false;

    std::string retval = octave::command_editor::readline (s, eof);

    if (! eof && retval.empty ())
      retval = "\n";

    return retval;
  }

  static void
  execute_in_debugger_handler (const std::pair<std::string, int>& arg)
  {
    octave_link::execute_in_debugger_event (arg.first, arg.second);
  }

  void input_system::get_debug_input (const std::string& prompt)
  {
    octave::unwind_protect frame;

    octave::tree_evaluator& tw = m_interpreter.get_evaluator ();

    bool silent = tw.quiet_breakpoint_flag (false);

    octave::call_stack& cs = m_interpreter.get_call_stack ();

    octave_user_code *caller = cs.caller_user_code ();
    std::string nm;
    int curr_debug_line;

    if (caller)
      {
        nm = caller->fcn_file_name ();

        if (nm.empty ())
          nm = caller->name ();

        curr_debug_line = cs.caller_user_code_line ();
      }
    else
      curr_debug_line = cs.current_line ();

    std::ostringstream buf;

    if (! nm.empty ())
      {
        if (m_gud_mode)
          {
            static char ctrl_z = 'Z' & 0x1f;

            buf << ctrl_z << ctrl_z << nm << ':' << curr_debug_line;
          }
        else
          {
            // FIXME: we should come up with a clean way to detect
            // that we are stopped on the no-op command that marks the
            // end of a function or script.

            if (! silent)
              {
                buf << "stopped in " << nm;

                if (curr_debug_line > 0)
                  buf << " at line " << curr_debug_line;
              }

            octave_link::enter_debugger_event (nm, curr_debug_line);

            octave_link::set_workspace ();

            frame.add_fcn (execute_in_debugger_handler,
                           std::pair<std::string, int> (nm, curr_debug_line));

            if (! silent)
              {
                std::string line_buf;

                if (caller)
                  line_buf = caller->get_code_line (curr_debug_line);

                if (! line_buf.empty ())
                  buf << "\n" << curr_debug_line << ": " << line_buf;
              }
          }
      }

    if (silent)
      octave::command_editor::erase_empty_line (true);

    std::string msg = buf.str ();

    if (! msg.empty ())
      std::cerr << msg << std::endl;

    frame.add_method (*this, &octave::input_system::set_PS1, m_PS1);
    m_PS1 = prompt;

    // FIXME: should debugging be possible in an embedded interpreter?

    octave::application *app = octave::application::app ();

    if (! app->interactive ())
      {

        frame.add_method (app, &octave::application::interactive,
                          app->interactive ());

        frame.add_method (app, &octave::application::forced_interactive,
                          app->forced_interactive ());

        app->interactive (true);

        app->forced_interactive (true);
      }

    octave::parser curr_parser (m_interpreter);

    while (Vdebugging)
      {
        try
          {
            Vtrack_line_num = false;

            reset_error_handler ();

            curr_parser.reset ();

            int retval = curr_parser.run ();

            if (octave::command_editor::interrupt (false))
              break;
            else
              {
                if (retval == 0 && curr_parser.m_stmt_list)
                  {
                    curr_parser.m_stmt_list->accept (tw);

                    if (octave_completion_matches_called)
                      octave_completion_matches_called = false;
                  }

                octave_quit ();
              }
          }
        catch (const octave::execution_exception& e)
          {
            std::string stack_trace = e.info ();

            if (! stack_trace.empty ())
              std::cerr << stack_trace;

            // Ignore errors when in debugging mode;
            octave::interpreter::recover_from_exception ();
          }
      }
  }

  std::string base_reader::octave_gets (bool& eof)
  {
    octave_quit ();

    eof = false;

    std::string retval;

    // Process pre input event hook function prior to flushing output and
    // printing the prompt.

    if (application::interactive ())
      {
        if (! Vdebugging)
          octave_link::exit_debugger_event ();

        octave_link::pre_input_event ();

        octave_link::set_workspace ();
      }

    bool history_skip_auto_repeated_debugging_command = false;

    input_system& input_sys = __get_input_system__ ("base_reader::octave_gets");

    std::string ps = (m_pflag > 0) ? input_sys.PS1 () : input_sys.PS2 ();

    std::string prompt = command_editor::decode_prompt_string (ps);

    pipe_handler_error_count = 0;

    output_system& output_sys = __get_output_system__ ("do_sync");

    output_sys.reset ();

    octave_diary << prompt;

    retval = input_sys.interactive_input (prompt, eof);

    // There is no need to update the load_path cache if there is no
    // user input.
    if (retval != "\n"
        && retval.find_first_not_of (" \t\n\r") != std::string::npos)
      {
        load_path& lp = __get_load_path__ ("base_reader::octave_gets");

        lp.update ();

        if (Vdebugging)
          input_sys.last_debugging_command (retval);
        else
          input_sys.last_debugging_command ("\n");
      }
    else if (Vdebugging)
      {
        retval = input_sys.last_debugging_command ();
        history_skip_auto_repeated_debugging_command = true;
      }

    if (retval != "\n")
      {
        if (! history_skip_auto_repeated_debugging_command)
          {
            if (command_history::add (retval))
              octave_link::append_history (retval);
          }

        octave_diary << retval;

        if (retval.back () != '\n')
          octave_diary << "\n";
      }
    else
      octave_diary << "\n";

    // Process post input event hook function after the internal history
    // list has been updated.

    if (application::interactive ())
      octave_link::post_input_event ();

    return retval;
  }

  bool base_reader::reading_fcn_file (void) const
  {
    return m_lexer ? m_lexer->m_reading_fcn_file : false;
  }

  bool base_reader::reading_classdef_file (void) const
  {
    return m_lexer ? m_lexer->m_reading_classdef_file : false;
  }

  bool base_reader::reading_script_file (void) const
  {
    return m_lexer ? m_lexer->m_reading_script_file : false;
  }

  class
  terminal_reader : public base_reader
  {
  public:

    terminal_reader (base_lexer *lxr = nullptr)
      : base_reader (lxr)
    { }

    std::string get_input (bool& eof);

    std::string input_source (void) const { return s_in_src; }

    bool input_from_terminal (void) const { return true; }

  private:

    static const std::string s_in_src;
  };

  class
  file_reader : public base_reader
  {
  public:

    file_reader (FILE *f_arg, base_lexer *lxr = nullptr)
      : base_reader (lxr), m_file (f_arg) { }

    std::string get_input (bool& eof);

    std::string input_source (void) const { return s_in_src; }

    bool input_from_file (void) const { return true; }

  private:

    FILE *m_file;

    static const std::string s_in_src;
  };

  class
  eval_string_reader : public base_reader
  {
  public:

    eval_string_reader (const std::string& str, base_lexer *lxr = nullptr)
      : base_reader (lxr), m_eval_string (str)
    { }

    std::string get_input (bool& eof);

    std::string input_source (void) const { return s_in_src; }

    bool input_from_eval_string (void) const { return true; }

  private:

    std::string m_eval_string;

    static const std::string s_in_src;
  };

  input_reader::input_reader (base_lexer *lxr)
    : m_rep (new terminal_reader (lxr))
  { }

  input_reader::input_reader (FILE *file, base_lexer *lxr)
    : m_rep (new file_reader (file, lxr))
  { }

  input_reader::input_reader (const std::string& str, base_lexer *lxr)
    : m_rep (new eval_string_reader (str, lxr))
  { }

  const std::string base_reader::s_in_src ("invalid");

  const std::string terminal_reader::s_in_src ("terminal");

  std::string
  terminal_reader::get_input (bool& eof)
  {
    octave_quit ();

    eof = false;

    return octave_gets (eof);
  }

  const std::string file_reader::s_in_src ("file");

  std::string
  file_reader::get_input (bool& eof)
  {
    octave_quit ();

    eof = false;

    std::string src_str = octave_fgets (m_file, eof);

    octave::input_system& input_sys
      = octave::__get_input_system__ ("get_input");

    std::string mfile_encoding = input_sys.mfile_encoding ();

    std::string encoding
      = (mfile_encoding.compare ("system") == 0
         ? octave_locale_charset_wrapper () : mfile_encoding);

    if (encoding.compare ("utf-8") != 0)
    {
      // convert encoding to UTF-8 before returning string
      const char *src = src_str.c_str ();
      size_t srclen = src_str.length ();

      size_t length;
      uint8_t *utf8_str = nullptr;

      utf8_str = octave_u8_conv_from_encoding (encoding.c_str (), src, srclen,
                                               &length);

      if (! utf8_str)
        error ("file_reader::get_input: converting from codepage '%s' to UTF-8: %s",
               encoding.c_str (), std::strerror (errno));

      octave::unwind_protect frame;
      frame.add_fcn (::free, static_cast<void *> (utf8_str));

      src_str = std::string (reinterpret_cast<char *> (utf8_str), length);
    }

    return src_str;
  }

  const std::string eval_string_reader::s_in_src ("eval_string");

  std::string
  eval_string_reader::get_input (bool& eof)
  {
    octave_quit ();

    eof = false;

    std::string retval;

    retval = m_eval_string;

    // Clear the eval string so that the next call will return
    // an empty character string with EOF = true.
    m_eval_string = "";

    if (retval.empty ())
      eof = true;

    return retval;
  }
}

DEFMETHOD (input, interp, args, nargout,
           doc: /* -*- texinfo -*-
@deftypefn  {} {@var{ans} =} input (@var{prompt})
@deftypefnx {} {@var{ans} =} input (@var{prompt}, "s")
Print @var{prompt} and wait for user input.

For example,

@example
input ("Pick a number, any number! ")
@end example

@noindent
prints the prompt

@example
Pick a number, any number!
@end example

@noindent
and waits for the user to enter a value.  The string entered by the user
is evaluated as an expression, so it may be a literal constant, a variable
name, or any other valid Octave code.

The number of return arguments, their size, and their class depend on the
expression entered.

If you are only interested in getting a literal string value, you can call
@code{input} with the character string @qcode{"s"} as the second argument.
This tells Octave to return the string entered by the user directly, without
evaluating it first.

Because there may be output waiting to be displayed by the pager, it is a
good idea to always call @code{fflush (stdout)} before calling @code{input}.
 This will ensure that all pending output is written to the screen before
your prompt.
@seealso{yes_or_no, kbhit, pause, menu, listdlg}
@end deftypefn */)
{
  int nargin = args.length ();

  if (nargin < 1 || nargin > 2)
    print_usage ();

  octave::input_system& input_sys = interp.get_input_system ();

  return input_sys.get_user_input (args, std::max (nargout, 1));
}

DEFMETHOD (yes_or_no, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {@var{ans} =} yes_or_no ("@var{prompt}")
Ask the user a yes-or-no question.

Return logical true if the answer is yes or false if the answer is no.

Takes one argument, @var{prompt}, which is the string to display when asking
the question.  @var{prompt} should end in a space; @code{yes-or-no} adds the
string @samp{(yes or no) } to it.  The user must confirm the answer with
@key{RET} and can edit it until it has been confirmed.
@seealso{input}
@end deftypefn */)
{
  int nargin = args.length ();

  if (nargin > 1)
    print_usage ();

  octave::input_system& input_sys = interp.get_input_system ();

  std::string prompt;

  if (nargin == 1)
    prompt = args(0).xstring_value ("yes_or_no: PROMPT must be a string");

  return ovl (input_sys.yes_or_no (prompt));
}

DEFMETHOD (keyboard, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn  {} {} keyboard ()
@deftypefnx {} {} keyboard ("@var{prompt}")
Stop m-file execution and enter debug mode.

When the @code{keyboard} function is executed, Octave prints a prompt and
waits for user input.  The input strings are then evaluated and the results
are printed.  This makes it possible to examine the values of variables
within a function, and to assign new values if necessary.  To leave the
prompt and return to normal execution type @samp{return} or @samp{dbcont}.
The @code{keyboard} function does not return an exit status.

If @code{keyboard} is invoked without arguments, a default prompt of
@samp{debug> } is used.
@seealso{dbstop, dbcont, dbquit}
@end deftypefn */)
{
  if (args.length () > 1)
    print_usage ();

  octave::unwind_protect frame;

  octave::call_stack& cs = interp.get_call_stack ();

  frame.add_method (cs, &octave::call_stack::restore_frame,
                    cs.current_frame ());

  // Skip the frame assigned to the keyboard function.
  cs.goto_frame_relative (0);

  octave::tree_evaluator& tw = interp.get_evaluator ();

  tw.debug_mode (true);
  tw.quiet_breakpoint_flag (false);
  tw.current_frame (cs.current_frame ());

  octave::input_system& input_sys = interp.get_input_system ();

  input_sys.keyboard (args);

  return ovl ();
}

DEFUN (completion_matches, args, nargout,
       doc: /* -*- texinfo -*-
@deftypefn {} {} completion_matches (@var{hint})
Generate possible completions given @var{hint}.

This function is provided for the benefit of programs like Emacs which
might be controlling Octave and handling user input.  The current
command number is not incremented when this function is called.  This is
a feature, not a bug.
@end deftypefn */)
{
  if (args.length () != 1)
    print_usage ();

  octave_value retval;

  std::string hint = args(0).string_value ();

  int n = 32;

  string_vector list (n);

  int k = 0;

  for (;;)
    {
      std::string cmd = generate_completion (hint, k);

      if (! cmd.empty ())
        {
          if (k == n)
            {
              n *= 2;
              list.resize (n);
            }

          list[k++] = cmd;
        }
      else
        {
          list.resize (k);
          break;
        }
    }

  if (nargout > 0)
    {
      if (! list.empty ())
        retval = list;
      else
        retval = "";
    }
  else
    {
      // We don't use string_vector::list_in_columns here
      // because it will be easier for Emacs if the names
      // appear in a single column.

      int len = list.numel ();

      for (int i = 0; i < len; i++)
        octave_stdout << list[i] << "\n";
    }

  octave_completion_matches_called = true;

  return retval;
}

/*
%!assert (ischar (completion_matches ("")))
%!assert (ischar (completion_matches ("a")))
%!assert (ischar (completion_matches (" ")))
%!assert (isempty (completion_matches (" ")))
%!assert (any (strcmp ("abs", deblank (cellstr (completion_matches (""))))))
%!assert (any (strcmp ("abs", deblank (cellstr (completion_matches ("a"))))))
%!assert (any (strcmp ("abs", deblank (cellstr (completion_matches ("ab"))))))
%!assert (any (strcmp ("abs", deblank (cellstr (completion_matches ("abs"))))))
%!assert (! any (strcmp ("abs", deblank (cellstr (completion_matches ("absa"))))))

%!error completion_matches ()
%!error completion_matches (1, 2)
*/

DEFUN (readline_read_init_file, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {} readline_read_init_file (@var{file})
Read the readline library initialization file @var{file}.

If @var{file} is omitted, read the default initialization file
(normally @file{~/.inputrc}).

@xref{Readline Init File, , , readline, GNU Readline Library},
for details.
@seealso{readline_re_read_init_file}
@end deftypefn */)
{
  int nargin = args.length ();

  if (nargin > 1)
    print_usage ();

  if (nargin == 0)
    octave::command_editor::read_init_file ();
  else
    {
      std::string file = args(0).string_value ();

      octave::command_editor::read_init_file (file);
    }

  return ovl ();
}

DEFUN (readline_re_read_init_file, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {} readline_re_read_init_file ()
Re-read the last readline library initialization file that was read.

@xref{Readline Init File, , , readline, GNU Readline Library},
for details.
@seealso{readline_read_init_file}
@end deftypefn */)
{
  if (args.length () != 0)
    print_usage ();

  octave::command_editor::re_read_init_file ();

  return ovl ();
}

static int
internal_input_event_hook_fcn (void)
{
  octave::input_system& input_sys
    = octave::__get_input_system__ ("internal_input_event_hook_fcn");

  input_sys.run_input_event_hooks ();

  if (! input_sys.have_input_event_hooks ())
    octave::command_editor::remove_event_hook (internal_input_event_hook_fcn);

  return 0;
}

DEFMETHOD (add_input_event_hook, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn  {} {@var{id} =} add_input_event_hook (@var{fcn})
@deftypefnx {} {@var{id} =} add_input_event_hook (@var{fcn}, @var{data})
Add the named function or function handle @var{fcn} to the list of functions
to call periodically when Octave is waiting for input.

The function should have the form

@example
@var{fcn} (@var{data})
@end example

If @var{data} is omitted, Octave calls the function without any arguments.

The returned identifier may be used to remove the function handle from the
list of input hook functions.
@seealso{remove_input_event_hook}
@end deftypefn */)
{
  int nargin = args.length ();

  if (nargin < 1 || nargin > 2)
    print_usage ();

  octave_value user_data;

  if (nargin == 2)
    user_data = args(1);

  octave::input_system& input_sys = interp.get_input_system ();

  hook_function hook_fcn (args(0), user_data);

  if (! input_sys.have_input_event_hooks ())
    octave::command_editor::add_event_hook (internal_input_event_hook_fcn);

  input_sys.add_input_event_hook (hook_fcn);

  return ovl (hook_fcn.id ());
}

DEFMETHOD (remove_input_event_hook, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn  {} {} remove_input_event_hook (@var{name})
@deftypefnx {} {} remove_input_event_hook (@var{fcn_id})
Remove the named function or function handle with the given identifier
from the list of functions to call periodically when Octave is waiting
for input.
@seealso{add_input_event_hook}
@end deftypefn */)
{
  int nargin = args.length ();

  if (nargin < 1 || nargin > 2)
    print_usage ();

  std::string hook_fcn_id = args(0).string_value ("remove_input_event_hook: argument not valid as a hook function name or id");

  bool warn = (nargin < 2);

  octave::input_system& input_sys = interp.get_input_system ();

  if (! input_sys.remove_input_event_hook (hook_fcn_id) && warn)
    warning ("remove_input_event_hook: %s not found in list",
             hook_fcn_id.c_str ());

  if (! input_sys.have_input_event_hooks ())
    octave::command_editor::remove_event_hook (internal_input_event_hook_fcn);

  return ovl ();
}

DEFMETHOD (PS1, interp, args, nargout,
           doc: /* -*- texinfo -*-
@deftypefn  {} {@var{val} =} PS1 ()
@deftypefnx {} {@var{old_val} =} PS1 (@var{new_val})
@deftypefnx {} {} PS1 (@var{new_val}, "local")
Query or set the primary prompt string.

When executing interactively, Octave displays the primary prompt when it is
ready to read a command.

The default value of the primary prompt string is @qcode{'octave:\#> '}.
To change it, use a command like

@example
PS1 ("\\u@@\\H> ")
@end example

@noindent
which will result in the prompt @samp{boris@@kremvax> } for the user
@samp{boris} logged in on the host @samp{kremvax.kgb.su}.  Note that two
backslashes are required to enter a backslash into a double-quoted
character string.  @xref{Strings}.

You can also use ANSI escape sequences if your terminal supports them.
This can be useful for coloring the prompt.  For example,

@example
PS1 ('\[\033[01;31m\]\s:\#> \[\033[0m\]')
@end example

@noindent
will give the default Octave prompt a red coloring.

When called from inside a function with the @qcode{"local"} option, the
variable is changed locally for the function and any subroutines it calls.
The original variable value is restored when exiting the function.
@seealso{PS2, PS4}
@end deftypefn */)
{
  octave::input_system& input_sys = interp.get_input_system ();

  return input_sys.PS1 (args, nargout);
}

DEFMETHOD (PS2, interp, args, nargout,
           doc: /* -*- texinfo -*-
@deftypefn  {} {@var{val} =} PS2 ()
@deftypefnx {} {@var{old_val} =} PS2 (@var{new_val})
@deftypefnx {} {} PS2 (@var{new_val}, "local")
Query or set the secondary prompt string.

The secondary prompt is printed when Octave is expecting additional input to
complete a command.  For example, if you are typing a @code{for} loop that
spans several lines, Octave will print the secondary prompt at the beginning
of each line after the first.  The default value of the secondary prompt
string is @qcode{"> "}.

When called from inside a function with the @qcode{"local"} option, the
variable is changed locally for the function and any subroutines it calls.
The original variable value is restored when exiting the function.
@seealso{PS1, PS4}
@end deftypefn */)
{
  octave::input_system& input_sys = interp.get_input_system ();

  return input_sys.PS2 (args, nargout);
}

DEFMETHOD (completion_append_char, interp, args, nargout,
           doc: /* -*- texinfo -*-
@deftypefn  {} {@var{val} =} completion_append_char ()
@deftypefnx {} {@var{old_val} =} completion_append_char (@var{new_val})
@deftypefnx {} {} completion_append_char (@var{new_val}, "local")
Query or set the internal character variable that is appended to
successful command-line completion attempts.

The default value is @qcode{" "} (a single space).

When called from inside a function with the @qcode{"local"} option, the
variable is changed locally for the function and any subroutines it calls.
The original variable value is restored when exiting the function.
@end deftypefn */)
{
  octave::input_system& input_sys = interp.get_input_system ();

  return input_sys.completion_append_char (args, nargout);
}

DEFMETHOD (__request_drawnow__, , args, ,
           doc: /* -*- texinfo -*-
@deftypefn  {} {} __request_drawnow__ ()
@deftypefnx {} {} __request_drawnow__ (@var{flag})
Undocumented internal function.
@end deftypefn */)
{
  int nargin = args.length ();

  if (nargin > 1)
    print_usage ();

  if (nargin == 0)
    Vdrawnow_requested = true;
  else
    Vdrawnow_requested = args(0).bool_value ();

  return ovl ();
}

DEFMETHOD (__gud_mode__, interp, args, nargout,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __gud_mode__ ()
Undocumented internal function.
@end deftypefn */)
{
  octave::input_system& input_sys = interp.get_input_system ();

  return input_sys.gud_mode (args, nargout);
}

DEFMETHOD (__mfile_encoding__, interp, args, nargout,
           doc: /* -*- texinfo -*-
@deftypefn {} {@var{current_encoding} =} __mfile_encoding__ (@var{new_encoding})
Set and query the codepage that is used for reading .m files.
@end deftypefn */)
{
  octave::input_system& input_sys = interp.get_input_system ();

  return input_sys.mfile_encoding (args, nargout);
}

#if defined (OCTAVE_USE_DEPRECATED_FUNCTIONS)

void
set_default_prompts (void)
{
  octave::input_system& input_sys
    = octave::__get_input_system__ ("set_default_prompts");

  input_sys.set_default_prompts ();
}

bool
octave_yes_or_no (const std::string& prompt)
{
  octave::input_system& input_sys
    = octave::__get_input_system__ ("set_default_prompts");

  input_sys.yes_or_no (prompt);
}

octave_value
do_keyboard (const octave_value_list& args)
{
  octave::input_system& input_sys
    = octave::__get_input_system__ ("do_keyboard");

  return input_sys.keyboard (args);
}

void
remove_input_event_hook_functions (void)
{
  octave::input_system& input_sys
    = octave::__get_input_system__ ("remove_input_event_hook_functions");

  input_sys.clear_input_event_hooks ();
}

// Fix things up so that input can come from the standard input.  This
// may need to become much more complicated, which is why it's in a
// separate function.

FILE *
get_input_from_stdin (void)
{
  octave::command_editor::set_input_stream (stdin);
  return octave::command_editor::get_input_stream ();
}

#endif
