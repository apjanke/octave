// help.cc                                             -*- C++ -*-
/*

Copyright (C) 1992, 1993, 1994, 1995 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <csignal>
#include <cstdlib>
#include <cstring>

#include <iostream.h>
#include <strstream.h>

#ifdef HAVE_UNISTD_H
#include <sys/types.h>
#include <unistd.h>
#endif

#include <readline/tilde.h>

#include "defun.h"
#include "dirfns.h"
#include "error.h"
#include "help.h"
#include "oct-obj.h"
#include "octave.h"
#include "pager.h"
#include "pathsearch.h"
#include "sighandlers.h"
#include "symtab.h"
#include "tree-const.h"
#include "tree-expr.h"
#include "tree-expr.h"
#include "user-prefs.h"
#include "utils.h"
#include "variables.h"

static help_list operators[] =
{
  { "!",
    "Logical not operator.  See also `~'.\n", },

  { "!=",
    "Logical not equals operator.  See also `~' and `<>'.\n", },

  { "\"",
    "String delimiter.\n", },

  { "#",
    "Begin comment character.  See also `%'.", },

  { "%",
    "Begin comment charcter.  See also `#'.", },

  { "&",
    "Logical and operator.  See also `&&'.", },

  { "&&",
    "Logical and operator.  See also `&'.", },

  { "'",
    "Matrix transpose operator.  For complex matrices, computes the\n\
complex conjugate (Hermitian) transpose.  See also `.''\n\
\n\
The single quote character may also be used to delimit strings, but\n\
it is better to use the double quote character, since that is never\n\
ambiguous", },

  { "(",
    "Array index or function argument delimiter.", },

  { ")",
    "Array index or function argument delimiter.", },

  { "*",
    "Multiplication operator.  See also `.*'", },

  { "**",
    "Power operator.  See also `^', `.**', and `.^'", },

  { "+",
    "Addition operator.", },

  { "++",
    "Increment operator.  As in C, may be applied as a prefix or postfix operator.", },

  { ",",
    "Array index, function argument, or command separator.", },

  { "-",
    "Subtraction or unary negation operator.", },

  { "--",
    "Decrement operator.  As in C, may be applied as a prefix or postfix operator.", },

  { ".'",
    "Matrix transpose operator.  For complex matrices, computes the\n\
transpose, *not* the complex conjugate transpose.  See also `''.", },

  { ".*",
    "Element by element multiplication operator.  See also `*'.", },

  { ".**",
    "Element by element power operator.  See also `**', `^', and `.^'.", },

  { "./",
    "Element by element division operator.  See also `/' and `\\'.", },

  { ".^",
    "Element by element power operator.  See also `**', `^', and `.^'.", },

  { "/",
    "Right division.  See also `\\' and `./'.", },

  { ":",
    "Select entire rows or columns of matrices.", },

  { ";",
    "Array row or command separator.  See also `,'.", },

  { "<",
    "Less than operator.", },

  { "<=",
    "Less than or equals operator.", },

  { "<>",
    "Logical not equals operator.  See also `!=' and `~='.", },

  { "=",
    "Assignment operator.", },

  { "==",
    "Equality test operator.", },

  { ">",
    "Greater than operator.", },

  { ">=",
    "Greater than or equals operator.", },

  { "[",
    "Return list delimiter.  See also `]'.", },

  { "\\",
    "Left division operator.  See also `/' and `./'.", },

  { "]",
    "Return list delimiter.  See also `['.", },

  { "^",
    "Power operator.  See also `**', `.^', and `.**.'", },

  { "|",
    "Logical or operator.  See also `||'.", },

  { "||",
    "Logical or operator.  See also `|'.", },

  { "~",
    "Logical not operator.  See also `!' and `~'.", },

  { "~=",
    "Logical not equals operator.  See also `<>' and `!='.", },

  { 0, 0, },
};

static help_list keywords[] =
{
  { "all_va_args",
    "Pass all unnamed arguments to another function call.", },

  { "break",
    "Exit the innermost enclosing while or for loop.", },

  { "catch",
    "begin the cleanup part of a try-catch block", },

  { "continue",
    "Jump to the end of the innermost enclosing while or for loop.", },

  { "else",
    "Alternate action for an if block.", },

  { "elseif",
    "Alternate conditional test for an if block.", },

  { "end",
    "Mark the end of any for, if, while, or function block.", },

  { "end_try_catch",
    "Mark the end of an try-catch block.", }, 

  { "end_unwind_protect",
    "Mark the end of an unwind_protect block.", }, 

  { "endfor",
    "Mark the end of a for loop.", },

  { "endfunction",
    "Mark the end of a function.", },

  { "endif",
    "Mark the end of an if block.", },

  { "endwhile",
    "Mark the end of a while loop.", },

  { "for",
    "Begin a for loop.", },

  { "function",
    "Begin a function body.", },

  { "global",
    "Declare variables to have global scope.", },

  { "gplot",
    "Produce 2-D plots using gnuplot-like command syntax.", },

  { "gsplot",
    "Produce 3-D plots using gnuplot-like command syntax.", },

  { "if",
    "Begin an if block.", },

  { "return",
    "Return from a function.", },

  { "try",
    "Begin a try-catch block.", }, 

  { "unwind_protect",
    "Begin an unwind_protect block.", }, 

  { "unwind_protect_cleanup",
    "Begin the cleanup section of an unwind_protect block.", }, 

  { "while",
    "Begin a while loop.", },

  { 0, 0, },
};

// Return a copy of the operator or keyword names.

char **
names (help_list *lst, int& count)
{
  count = 0;
  help_list *ptr = lst;
  while (ptr->name)
    {
      count++;
      ptr++;
    }

  if (count == 0)
    return 0;
    
  char **name_list = new char * [count+1];

  ptr = lst;
  int i = 0;
  while (ptr->name)
    {
      name_list[i++] = strsave (ptr->name);
      ptr++;
    }

  name_list[i] = 0;
  return name_list;
}

help_list *
operator_help (void)
{
  return operators;
}

help_list *
keyword_help (void)
{
  return keywords;
}

#define VERBOSE_HELP_MESSAGE \
  "\n\
Additional help for builtin functions, operators, and variables\n\
is available in the on-line version of the manual.\n\
\n\
Use the command `help -i <topic>' to search the manual index.\n"

static void
additional_help_message (ostrstream& output_buf)
{
#ifdef USE_GNU_INFO
  if (! user_pref.suppress_verbose_help_message)
    output_buf << VERBOSE_HELP_MESSAGE;
#endif
}

void
print_usage (const char *string, int just_usage)
{
  ostrstream output_buf;

  symbol_record *sym_rec = global_sym_tab->lookup (string, 0, 0);
  if (sym_rec)
    {
      char *h = sym_rec->help ();
      if (h && *h)
	{
	  output_buf << "\n*** " << string << ":\n\n"
	    << h << "\n";

	  if (! just_usage)
	    additional_help_message (output_buf);
	  output_buf << ends;
	  maybe_page_output (output_buf);
	}
    }
  else
    warning ("no usage message found for `%s'", string);
}

static void
display_names_from_help_list (ostrstream& output_buf, help_list *list,
			      const char *desc)
{
  int count = 0;
  char **symbols = names (list, count);
  output_buf << "\n*** " << desc << ":\n\n";
  if (symbols && count > 0)
    list_in_columns (output_buf, symbols);
  delete [] symbols;
}

static char *
print_symbol_type (ostrstream& output_buf, symbol_record *sym_rec,
		   char *name, int print)
{
  char *retval = 0;

  if (sym_rec->is_user_function ())
    {
      tree_fvc *defn = sym_rec->def ();
      char *fn = defn->fcn_file_name ();
      if (fn)
	{
	  char *ff = fcn_file_in_path (fn);
	  ff = ff ? ff : fn;

	  if (print)
	    output_buf << name
	      << " is the function defined from:\n"
		<< ff << "\n";
	  else
	    retval = ff;
	}
      else
	{
	  if (print)
	    output_buf << name << " is a user-defined function\n";
	  else
	    retval = "user-defined function";
	}
    }
  else if (sym_rec->is_text_function ())
    {
      if (print)
	output_buf << name << " is a builtin text-function\n";
      else
	retval = "builtin text-function";
    }
  else if (sym_rec->is_builtin_function ())
    {
      if (print)
	output_buf << name << " is a builtin function\n";
      else
	retval = "builtin function";
    }
  else if (sym_rec->is_user_variable ())
    {
      if (print)
	output_buf << name << " is a user-defined variable\n";
      else
	retval = "user-defined variable";
    }
  else if (sym_rec->is_builtin_variable ())
    {
      if (print)
	output_buf << name << " is a builtin variable\n";
      else
	retval = "builtin variable";
    }
  else
    {
      if (print)
	output_buf << "which: `" << name
	  << "' has unknown type\n";
      else
	retval = "unknown type";
    }

  return retval;
}

static void
display_symtab_names (ostrstream& output_buf, char **names,
		      int count, const char *desc)
{
  output_buf << "\n*** " << desc << ":\n\n";
  if (names && count > 0)
    list_in_columns (output_buf, names);
}

static void
simple_help (void)
{
  ostrstream output_buf;

  display_names_from_help_list (output_buf, operator_help (),
				"operators");

  display_names_from_help_list (output_buf, keyword_help (),
				"reserved words");

#ifdef LIST_SYMBOLS
#undef LIST_SYMBOLS
#endif
#define LIST_SYMBOLS(type, msg) \
  do \
    { \
      int count; \
      char **names = global_sym_tab->list (count, 0, 0, 1, type); \
      display_symtab_names (output_buf, names, count, msg); \
      char **ptr = names; \
      while (*ptr) \
        delete [] *ptr++; \
      delete [] names; \
    } \
  while (0)

  // XXX FIXME XXX -- is this distinction needed?

  LIST_SYMBOLS (symbol_def::TEXT_FUNCTION,
		"text functions (these names are also reserved)");

  LIST_SYMBOLS (symbol_def::MAPPER_FUNCTION, "mapper functions");

  LIST_SYMBOLS (symbol_def::BUILTIN_FUNCTION, "general functions");

  LIST_SYMBOLS (symbol_def::BUILTIN_VARIABLE, "builtin variables");

  // Also need to list variables and currently compiled functions from
  // the symbol table, if there are any.

  // Also need to search octave_path for script files.

  char *path_elt = kpse_path_element (user_pref.loadpath);

  while (path_elt)
    {
      str_llist_type *elt_dirs = kpse_element_dirs (path_elt);

      str_llist_elt_type *dir;
      for (dir = *elt_dirs; dir; dir = STR_LLIST_NEXT (*dir))
	{
	  char *elt_dir = STR_LLIST (*dir);

	  if (elt_dir)
	    {
	      int count;
	      char **names = get_fcn_file_names (count, elt_dir, 0);

	      output_buf << "\n*** function files in "
		<< make_absolute (elt_dir, the_current_working_directory)
		  << ":\n\n";

	      if (names && count > 0)
		list_in_columns (output_buf, names);

	      delete [] names;
	    }
	}

      path_elt = kpse_path_element (0);
    }

  additional_help_message (output_buf);
  output_buf << ends;
  maybe_page_output (output_buf);
}

#ifdef USE_GNU_INFO
static int
try_info (const char *string)
{
  int status = 0;

  static char *cmd_str = 0;

  delete [] cmd_str;
  cmd_str = 0;

  ostrstream cmd_buf;

  cmd_buf << "info --file " << user_pref.info_file;

  char *directory_name = strsave (user_pref.info_file);
  char *file = strrchr (directory_name, '/');
  if (file)
    {
      file++;
      *file = 0;
      cmd_buf << " --directory " << directory_name;
    }
  delete [] directory_name;

  if (string)
    cmd_buf << " --index-search " << string;

  cmd_buf << ends;

  cmd_str = cmd_buf.str ();

  volatile sig_handler *old_sigint_handler;
  old_sigint_handler = octave_set_signal_handler (SIGINT, SIG_IGN);

  status = system (cmd_str);

  octave_set_signal_handler (SIGINT, old_sigint_handler);

  if ((status & 0xff) == 0)
    status = (signed char) ((status & 0xff00) >> 8);
  else
    status = 127;

  return status;
}
#endif

static void
help_from_info (int argc, char **argv)
{
#ifdef USE_GNU_INFO
  if (argc == 1)
    try_info (0);
  else
    {
      while (--argc > 0)
	{
	  argv++;

	  if (! *argv || ! **argv)
	    continue;

	  int status = try_info (*argv);

	  if (status)
	    {
	      if (status < 0)
		{
		  message ("help",
			   "sorry, `%s' is not indexed in the manual",
			   *argv);
		  sleep (2);
		}
	      else
		{
		  error ("help: unable to find info!");
		  break;
		}
	    }
	}
    }
#else
  message (0, "sorry, help -i is not available in this version of Octave");
#endif
}

int
help_from_list (ostrstream& output_buf, const help_list *list,
		const char *string, int usage)
{
  char *name;
  while ((name = list->name) != 0)
    {
      if (strcmp (name, string) == 0)
	{
	  if (usage)
	    output_buf << "\nusage: ";
	  else
	    {
	      output_buf << "\n*** " << string << ":\n\n";
	    }

	  output_buf << list->help << "\n";

	  return 1;
	}
      list++;
    }
  return 0;
}

static void
builtin_help (int argc, char **argv)
{
  ostrstream output_buf;

  help_list *op_help_list = operator_help ();
  help_list *kw_help_list = keyword_help ();

  while (--argc > 0)
    {
      argv++;

      if (! *argv || ! **argv)
	continue;

      if (help_from_list (output_buf, op_help_list, *argv, 0))
	continue;

      if (help_from_list (output_buf, kw_help_list, *argv, 0))
	continue;

      symbol_record *sym_rec = lookup_by_name (*argv, 0);

      if (sym_rec && sym_rec->is_defined ())
	{
	  char *h = sym_rec->help ();
	  if (h && *h)
	    {
	      print_symbol_type (output_buf, sym_rec, *argv, 1);
	      output_buf << "\n" << h << "\n";
	      continue;
	    }
	}

      char *path = fcn_file_in_path (*argv);
      char *h = get_help_from_file (path);
      if (h && *h)
	{
	  output_buf << *argv << " is the file:\n"
	    << path << "\n\n" << h << "\n";
	  delete [] h;
	  delete [] path;
	  continue;
	}
      delete [] path;

      output_buf << "\nhelp: sorry, `" << *argv << "' is not documented\n"; 
    }

  additional_help_message (output_buf);
  output_buf << ends;
  maybe_page_output (output_buf);
}

#ifdef USE_GNU_INFO
DEFUN_TEXT ("help", Fhelp, Shelp, 10,
  "help [-i] [topic ...]\n\
\n\
print cryptic yet witty messages")
#else
DEFUN_TEXT ("help", Fhelp, Shelp, 10,
  "help [topic ...]\n\
\n\
print cryptic yet witty messages")
#endif
{
  Octave_object retval;

  DEFINE_ARGV("help");

  if (argc == 1)
    {
      simple_help ();
    }
  else
    {
      if (argv[1] && strcmp (argv[1], "-i") == 0)
	{
	  argc--;
	  argv++;

	  help_from_info (argc, argv);
	}
      else
	{
	  builtin_help (argc, argv);
	}
    }

  DELETE_ARGV;

  return retval;
}

DEFUN_TEXT ("type", Ftype, Stype, 11,
  "type NAME ...]\n\
\n\
display the definition of each NAME that refers to a function")
{
  Octave_object retval;

  DEFINE_ARGV("type");

  if (argc > 1)
    {
      // XXX FIXME XXX -- we should really use getopt ()

      int quiet = 0;
      if (argv[1] && strcmp (argv[1], "-q") == 0)
	{
	  quiet = 1;
	  argc--;
	  argv++;
	}

      ostrstream output_buf;

      while (--argc > 0)
	{
	  argv++;

	  if (! *argv || ! **argv)
	    continue;

	  char *id = strsave (*argv);
	  char *elts = 0;
	  if (id[strlen (id) - 1] != '.')
	    {
	      char *ptr = strchr (id, '.');
	      if (ptr)
		{
		  *ptr = '\0';
		  elts = ptr + 1;
		}
	    }

	  symbol_record *sym_rec = lookup_by_name (id, 0);

	  if (sym_rec)
	    {
	      if (sym_rec->is_user_function ())
		{
		  tree_fvc *defn = sym_rec->def ();

		  if (nargout == 0 && ! quiet)
		    output_buf << *argv << " is a user-defined function\n";

		  defn->print_code (output_buf);
		}

	      // XXX FIXME XXX -- this code should be shared with
	      // Fwhich.

	      else if (sym_rec->is_text_function ())
		output_buf << *argv << " is a builtin text-function\n";
	      else if (sym_rec->is_builtin_function ())
		output_buf << *argv << " is a builtin function\n";
	      else if (sym_rec->is_user_variable ()
		       || sym_rec->is_builtin_variable ())
		{
		  tree_fvc *defn = sym_rec->def ();

		  assert (defn && defn->is_constant ());

		  tree_constant *tmp = (tree_constant *) defn;

		  int var_ok = 1;
		  if (tmp && tmp->is_map ())
		    {
		      if (elts && *elts)
			{
			  tree_constant ult =
			    tmp->lookup_map_element (elts, 0, 1);

			  if (! ult.is_defined ())
			    var_ok = 0;			    
			}
		    }

		  if (nargout == 0 && ! quiet)
		    {
		      if (var_ok)
			{
			  output_buf << *argv;
			  if (sym_rec->is_user_variable ())
			    output_buf << " is a user-defined variable\n";
			  else
			    output_buf << " is a built-in variable\n";
			}
		      else
			{
			  if (elts && *elts)
			    output_buf << "type: structure `" << id
			      << "' has no member `" << elts << "'\n";
			  else
			    output_buf << "type: `" << id
			      << "' has unknown type!";
			}
		    }
		  if (! tmp->is_map ())
		    {
		      tmp->print_code (output_buf);

		      if (nargout == 0)
			output_buf << "\n";
		    }
		}
	      else
		output_buf << "type: `" << *argv << "' has unknown type!\n";
	    }
	  else
	    output_buf << "type: `" << *argv << "' undefined\n";

	  delete [] id;
	}

      output_buf << ends;

      if (nargout == 0)
	maybe_page_output (output_buf);
      else
	{
	  char *s = output_buf.str ();
	  retval = s;
	  delete [] s;
	}
    }
  else
    print_usage ("type");

  DELETE_ARGV;

  return retval;
}

DEFUN_TEXT ("which", Fwhich, Swhich, 11,
  "which NAME ...]\n\
\n\
display the type of each NAME.  If NAME is defined from an function\n\
file, print the full name of the file.")
{
  Octave_object retval;

  DEFINE_ARGV("which");

  if (argc > 1)
    {
      if (nargout > 0)
	retval.resize (argc-1, Matrix ());

      ostrstream output_buf;

      for (int i = 0; i < argc-1; i++)
	{
	  argv++;

	  if (! *argv || ! **argv)
	    continue;

	  symbol_record *sym_rec = lookup_by_name (*argv, 0);

	  if (sym_rec)
	    {
	      int print = (nargout == 0);
	      char *tmp = print_symbol_type (output_buf, sym_rec,
					     *argv, print);
	      if (! print)
		retval(i) = tmp;
	    }
	  else
	    {
	      if (nargout == 0)
		output_buf << "which: `" << *argv << "' is undefined\n";
	      else
		retval(i) = "undefined";
	    }
	}
      output_buf << ends;
      maybe_page_output (output_buf);
    }
  else
    print_usage ("which");

  DELETE_ARGV;

  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; page-delimiter: "^/\\*" ***
;;; End: ***
*/
