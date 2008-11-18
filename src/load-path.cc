/*

Copyright (C) 2006, 2007 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <algorithm>

#include "dir-ops.h"
#include "file-ops.h"
#include "file-stat.h"
#include "oct-env.h"
#include "pathsearch.h"

#include "defaults.h"
#include "defun.h"
#include "input.h"
#include "load-path.h"
#include "pager.h"
#include "parse.h"
#include "toplev.h"
#include "unwind-prot.h"
#include "utils.h"

load_path *load_path::instance = 0;
load_path::hook_fcn_ptr load_path::add_hook = execute_pkg_add;
load_path::hook_fcn_ptr load_path::remove_hook = execute_pkg_del;
std::string load_path::command_line_path;
std::string load_path::sys_path;
load_path::abs_dir_cache_type load_path::abs_dir_cache;

void
load_path::dir_info::update (void)
{
  file_stat fs (dir_name);

  if (fs)
    {
      if (is_relative)
	{
	  std::string abs_name
	    = octave_env::make_absolute (dir_name, octave_env::getcwd ());

	  abs_dir_cache_iterator p = abs_dir_cache.find (abs_name);

	  if (p != abs_dir_cache.end ())
	    {
	      // The directory is in the cache of all directories we have
	      // visited (indexed by its absolute name).  If it is out of
	      // date, initialize it.  Otherwise, copy the info from the
	      // cache.  By doing that, we avoid unnecessary calls to stat
	      // that can slow things down tremendously for large
	      // directories.

	      const dir_info& di = p->second;

	      if (fs.mtime () != di.dir_mtime)
		initialize ();
	      else
		*this = di;

	      return;
	    }
	}

      if (fs.mtime () != dir_mtime)
	initialize ();
    }
  else
    {
      std::string msg = fs.error ();
      warning ("load_path: %s: %s", dir_name.c_str (), msg.c_str ());
    }
}

void
load_path::dir_info::initialize (void)
{
  is_relative = ! octave_env::absolute_pathname (dir_name);

  file_stat fs (dir_name);

  if (fs)
    {
      dir_mtime = fs.mtime ();

      get_file_list (dir_name);

      std::string abs_name
	= octave_env::make_absolute (dir_name, octave_env::getcwd ());

      // FIXME -- nothing is ever removed from this cache of directory
      // information, so there could be some resource problems.
      // Perhaps it should be pruned from time to time.

      abs_dir_cache[abs_name] = *this;      
    }
  else
    {
      std::string msg = fs.error ();
      warning ("load_path: %s: %s", dir_name.c_str (), msg.c_str ());
    }
}

void
load_path::dir_info::get_file_list (const std::string& d)
{
  dir_entry dir (d);

  if (dir)
    {
      string_vector flist = dir.read ();

      octave_idx_type len = flist.length ();

      all_files.resize (len);
      fcn_files.resize (len);

      octave_idx_type all_files_count = 0;
      octave_idx_type fcn_files_count = 0;

      for (octave_idx_type i = 0; i < len; i++)
	{
	  std::string fname = flist[i];

	  std::string full_name = file_ops::concat (d, fname);

	  file_stat fs (full_name);

	  if (fs)
	    {
	      if (fs.is_dir ())
		{
		  if (fname == "private")
		    get_private_file_map (full_name);
		  else if (fname[0] == '@')
		    get_method_file_map (full_name, fname.substr (1));
		}
	      else
		{
		  all_files[all_files_count++] = fname;

		  size_t pos = fname.rfind ('.');

		  if (pos != std::string::npos)
		    {
		      std::string ext = fname.substr (pos);

		      if (ext == ".m" || ext == ".oct" || ext == ".mex")
			{
			  std::string base = fname.substr (0, pos);

			  if (valid_identifier (base))
			    fcn_files[fcn_files_count++] = fname;
			}
		    }
		}
	    }
	}

      all_files.resize (all_files_count);
      fcn_files.resize (fcn_files_count);
    }
  else
    {
      std::string msg = dir.error ();
      warning ("load_path: %s: %s", d.c_str (), msg.c_str ());
    }
}

load_path::dir_info::fcn_file_map_type
get_fcn_files (const std::string& d)
{
  load_path::dir_info::fcn_file_map_type retval;

  dir_entry dir (d);

  if (dir)
    {
      string_vector flist = dir.read ();

      octave_idx_type len = flist.length ();

      for (octave_idx_type i = 0; i < len; i++)
	{
	  std::string fname = flist[i];

	  std::string ext;
	  std::string base = fname;

	  size_t pos = fname.rfind ('.');

	  if (pos != std::string::npos)
	    {
	      base = fname.substr (0, pos);
	      ext = fname.substr (pos);

	      if (valid_identifier (base))
		{
		  int t = 0;

		  if (ext == ".m")
		    t = load_path::M_FILE;
		  else if (ext == ".oct")
		    t = load_path::OCT_FILE;
		  else if (ext == ".mex")
		    t = load_path::MEX_FILE;

		  retval[base] |= t;
		}
	    }
	}
    }
  else
    {
      std::string msg = dir.error ();
      warning ("load_path: %s: %s", d.c_str (), msg.c_str ());
    }

  return retval;
}

void
load_path::dir_info::get_private_file_map (const std::string& d)
{
  private_file_map = get_fcn_files (d);
}

void
load_path::dir_info::get_method_file_map (const std::string& d,
					  const std::string& class_name)
{
  method_file_map[class_name].method_file_map = get_fcn_files (d);

  std::string pd = file_ops::concat (d, "private");

  file_stat fs (pd);

  if (fs && fs.is_dir ())
    method_file_map[class_name].private_file_map = get_fcn_files (pd);
}

bool
load_path::instance_ok (void)
{
  bool retval = true;

  if (! instance)
    instance = new load_path ();

  if (! instance)
    {
      ::error ("unable to create load path object!");

      retval = false;
    }

  return retval;
}

// FIXME -- maybe we should also maintain a map to speed up this
// method of access.

load_path::const_dir_info_list_iterator
load_path::find_dir_info (const std::string& dir_arg) const
{
  std::string dir = file_ops::tilde_expand (dir_arg);

  const_dir_info_list_iterator retval = dir_info_list.begin ();

  while (retval != dir_info_list.end ())
    {
      if (retval->dir_name == dir)
	break;

      retval++;
    }

  return retval;
}

load_path::dir_info_list_iterator
load_path::find_dir_info (const std::string& dir_arg)
{
  std::string dir = file_ops::tilde_expand (dir_arg);

  dir_info_list_iterator retval = dir_info_list.begin ();

  while (retval != dir_info_list.end ())
    {
      if (retval->dir_name == dir)
	break;

      retval++;
    }

  return retval;
}

bool
load_path::contains (const std::string& dir) const
{
  return find_dir_info (dir) != dir_info_list.end ();
}

void
load_path::move_fcn_map (const std::string& dir_name,
			 const string_vector& fcn_files, bool at_end)
{
  octave_idx_type len = fcn_files.length ();

  for (octave_idx_type k = 0; k < len; k++)
    {
      std::string fname = fcn_files[k];

      std::string ext;
      std::string base = fname;

      size_t pos = fname.rfind ('.');

      if (pos != std::string::npos)
	{
	  base = fname.substr (0, pos);
	  ext = fname.substr (pos);
	}

      file_info_list_type& file_info_list = fcn_map[base];

      if (file_info_list.size () == 1)
	continue;
      else
	{
	  for (file_info_list_iterator p = file_info_list.begin ();
	       p != file_info_list.end ();
	       p++)
	    {
	      if (p->dir_name == dir_name)
		{
		  file_info fi = *p;

		  file_info_list.erase (p);

		  if (at_end)
		    file_info_list.push_back (fi);
		  else
		    file_info_list.push_front (fi);

		  break;
		}
	    }
	}
    }
}

void
load_path::move_method_map (const std::string& dir_name, bool at_end)
{
  for (method_map_iterator i = method_map.begin ();
       i != method_map.end ();
       i++)
    {
      std::string class_name = i->first;

      fcn_map_type& fm = i->second;

      std::string full_dir_name
	= file_ops::concat (dir_name, "@" + class_name);

      for (fcn_map_iterator q = fm.begin (); q != fm.end (); q++)
	{
	  file_info_list_type& file_info_list = q->second;

	  if (file_info_list.size () == 1)
	    continue;
	  else
	    {
	      for (file_info_list_iterator p = file_info_list.begin ();
	       p != file_info_list.end ();
	       p++)
		{
		  if (p->dir_name == full_dir_name)
		    {
		      file_info fi = *p;

		      file_info_list.erase (p);

		      if (at_end)
			file_info_list.push_back (fi);
		      else
			file_info_list.push_front (fi);

		      break;
		    }
		}
	    }
	}
    }
}

void
load_path::move (dir_info_list_iterator i, bool at_end)
{
  if (dir_info_list.size () > 1)
    {
      dir_info di = *i;

      dir_info_list.erase (i);

      if (at_end)
	dir_info_list.push_back (di);
      else
	dir_info_list.push_front (di);

      std::string dir_name = di.dir_name;

      move_fcn_map (dir_name, di.fcn_files, at_end);

      // No need to move elements of private function map.

      move_method_map (dir_name, at_end);
    }
}

static void
maybe_add_path_elts (std::string& path, const std::string& dir)
{
  std::string tpath = genpath (dir);

  if (! tpath.empty ())
    {
      if (path.empty ())
	path = tpath;
      else
	path += dir_path::path_sep_str () + tpath;
    }
}

void
load_path::do_initialize (bool set_initial_path)
{
  sys_path = "";

  if (set_initial_path)
    {
      maybe_add_path_elts (sys_path, Vlocal_ver_oct_file_dir);
      maybe_add_path_elts (sys_path, Vlocal_api_oct_file_dir);
      maybe_add_path_elts (sys_path, Vlocal_oct_file_dir);
      maybe_add_path_elts (sys_path, Vlocal_ver_fcn_file_dir);
      maybe_add_path_elts (sys_path, Vlocal_api_fcn_file_dir);
      maybe_add_path_elts (sys_path, Vlocal_fcn_file_dir);
      maybe_add_path_elts (sys_path, Voct_file_dir);
      maybe_add_path_elts (sys_path, Vfcn_file_dir);
    }

  std::string tpath = load_path::command_line_path;

  if (tpath.empty ())
    tpath = octave_env::getenv ("OCTAVE_PATH");

  std::string xpath = ".";

  if (! tpath.empty ())
    xpath += dir_path::path_sep_str () + tpath;

  if (! sys_path.empty ())
    xpath += dir_path::path_sep_str () + sys_path;

  do_set (xpath, false);
}

void
load_path::do_clear (void)
{
  dir_info_list.clear ();
  fcn_map.clear ();
  private_fcn_map.clear ();
  method_map.clear ();

  do_append (".", false);
}

static std::list<std::string>
split_path (const std::string& p)
{
  std::list<std::string> retval;

  size_t beg = 0;
  size_t end = p.find (dir_path::path_sep_char ());

  size_t len = p.length ();

  while (end != std::string::npos)
    {
      std::string elt = p.substr (beg, end-beg);

      if (! elt.empty ())
	retval.push_back (elt);

      beg = end + 1;

      if (beg == len)
	break;

      end = p.find (dir_path::path_sep_char (), beg);
    }

  std::string elt = p.substr (beg);

  if (! elt.empty ())
    retval.push_back (elt);

  return retval;
}

void
load_path::do_set (const std::string& p, bool warn)
{
  do_clear ();

  std::list<std::string> elts = split_path (p);

  // Temporarily disable add hook.

  unwind_protect_fptr (add_hook);

  add_hook = 0;

  for (std::list<std::string>::const_iterator i = elts.begin ();
       i != elts.end ();
       i++)
    do_append (*i, warn);

  // Restore add hook and execute for all newly added directories.

  unwind_protect::run ();

  for (dir_info_list_iterator i = dir_info_list.begin ();
       i != dir_info_list.end ();
       i++)
    {
      if (add_hook)
	add_hook (i->dir_name);
    }
}

void
load_path::do_append (const std::string& dir, bool warn)
{
  if (! dir.empty ())
    do_add (dir, true, warn);
}

void
load_path::do_prepend (const std::string& dir, bool warn)
{
  if (! dir.empty ())
    do_add (dir, false, warn);
}

void
load_path::do_add (const std::string& dir_arg, bool at_end, bool warn)
{
  size_t len = dir_arg.length ();

  if (len > 1 && dir_arg.substr (len-2) == "//")
    warning_with_id ("Octave:recursive-path-search",
		     "trailing `//' is no longer special in search path elements");

  std::string dir = file_ops::tilde_expand (dir_arg);

  dir_info_list_iterator i = find_dir_info (dir);

  if (i != dir_info_list.end ())
    move (i, at_end);
  else
    {
      file_stat fs (dir);

      if (fs)
	{
	  if (fs.is_dir ())
	    {
	      dir_info di (dir);

	      if (! error_state)
		{
		  if (at_end)
		    dir_info_list.push_back (di);
		  else
		    dir_info_list.push_front (di);

		  add_to_fcn_map (di, true);

		  add_to_private_fcn_map (di);

		  add_to_method_map (di, true);

		  if (add_hook)
		    add_hook (dir);
		}
	    }
	  else if (warn)
	    warning ("addpath: %s: not a directory", dir_arg.c_str ());
	}
      else if (warn)
	{
	  std::string msg = fs.error ();
	  warning ("addpath: %s: %s", dir_arg.c_str (), msg.c_str ());
	}
    }

  // FIXME -- is there a better way to do this?

  i = find_dir_info (".");

  if (i != dir_info_list.end ())
    move (i, false);
  else
    panic_impossible ();
}

void
load_path::remove_fcn_map (const std::string& dir,
			   const string_vector& fcn_files)
{
  octave_idx_type len = fcn_files.length ();

  for (octave_idx_type k = 0; k < len; k++)
    {
      std::string fname = fcn_files[k];

      std::string ext;
      std::string base = fname;

      size_t pos = fname.rfind ('.');

      if (pos != std::string::npos)
	{
	  base = fname.substr (0, pos);
	  ext = fname.substr (pos);
	}

      file_info_list_type& file_info_list = fcn_map[base];

      for (file_info_list_iterator p = file_info_list.begin ();
	   p != file_info_list.end ();
	   p++)
	{
	  if (p->dir_name == dir)
	    {
	      file_info_list.erase (p);

	      if (file_info_list.empty ())
		fcn_map.erase (fname);

	      break;
	    }
	}
    }
}

void
load_path::remove_private_fcn_map (const std::string& dir)
{
  private_fcn_map_iterator p = private_fcn_map.find (dir);

  if (p != private_fcn_map.end ())
    private_fcn_map.erase (p);
}

void
load_path::remove_method_map (const std::string& dir)
{
  for (method_map_iterator i = method_map.begin ();
       i != method_map.end ();
       i++)
    {
      std::string class_name = i->first;

      fcn_map_type& fm = i->second;

      std::string full_dir_name = file_ops::concat (dir, "@" + class_name);

      for (fcn_map_iterator q = fm.begin (); q != fm.end (); q++)
	{
	  file_info_list_type& file_info_list = q->second;

	  if (file_info_list.size () == 1)
	    continue;
	  else
	    {
	      for (file_info_list_iterator p = file_info_list.begin ();
	       p != file_info_list.end ();
	       p++)
		{
		  if (p->dir_name == full_dir_name)
		    {
		      file_info_list.erase (p);

		      // FIXME -- if there are no other elements, we
		      // should remove this element of fm but calling
		      // erase here would invalidate the iterator q.

		      break;
		    }
		}
	    }
	}
    }
}

bool
load_path::do_remove (const std::string& dir_arg)
{
  bool retval = false;

  if (! dir_arg.empty ())
    {
      if (dir_arg == ".")
	{
	  warning ("rmpath: can't remove \".\" from path");

	  // Avoid additional warnings.
	  retval = true;
	}
      else
	{
	  std::string dir = file_ops::tilde_expand (dir_arg);

	  dir_info_list_iterator i = find_dir_info (dir);

	  if (i != dir_info_list.end ())
	    {
	      retval = true;

	      if (remove_hook)
		remove_hook (dir);

	      string_vector fcn_files = i->fcn_files;

	      dir_info_list.erase (i);

	      remove_fcn_map (dir, fcn_files);

	      remove_private_fcn_map (dir);

	      remove_method_map (dir);
	    }
	}
    }

  return retval;
}

void
load_path::do_update (void) const
{
  // I don't see a better way to do this because we need to
  // preserve the correct directory ordering for new files that
  // have appeared.

  fcn_map.clear ();

  private_fcn_map.clear ();

  method_map.clear ();

  for (dir_info_list_iterator p = dir_info_list.begin ();
       p != dir_info_list.end ();
       p++)
    {
      dir_info& di = *p;

      di.update ();

      add_to_fcn_map (di, true);

      add_to_private_fcn_map (di);

      add_to_method_map (di, true);
    }
}

bool
load_path::check_file_type (std::string& fname, int type, int possible_types,
			    const std::string& fcn, const char *who)
{
  bool retval = false;

  if (type == load_path::OCT_FILE)
    {
      if ((type & possible_types) == load_path::OCT_FILE)
	{
	  fname += ".oct";
	  retval = true;
	}
    }
  else if (type == load_path::M_FILE)
    {
      if ((type & possible_types) == load_path::M_FILE)
	{
	  fname += ".m";
	  retval = true;
	}
    }
  else if (type == load_path::MEX_FILE)
    {
      if ((type & possible_types) == load_path::MEX_FILE)
	{
	  fname += ".mex";
	  retval = true;
	}
    }
  else if (type == (load_path::M_FILE | load_path::OCT_FILE))
    {
      if (possible_types & load_path::OCT_FILE)
	{
	  fname += ".oct";
	  retval = true;
	}
      else if (possible_types & load_path::M_FILE)
	{
	  fname += ".m";
	  retval = true;
	}
    }
  else if (type == (load_path::M_FILE | load_path::MEX_FILE))
    {
      if (possible_types & load_path::MEX_FILE)
	{
	  fname += ".mex";
	  retval = true;
	}
      else if (possible_types & load_path::M_FILE)
	{
	  fname += ".m";
	  retval = true;
	}
    }
  else if (type == (load_path::OCT_FILE | load_path::MEX_FILE))
    {
      if (possible_types & load_path::OCT_FILE)
	{
	  fname += ".oct";
	  retval = true;
	}
      else if (possible_types & load_path::MEX_FILE)
	{
	  fname += ".mex";
	  retval = true;
	}
    }
  else if (type == (load_path::M_FILE | load_path::OCT_FILE
		    | load_path::MEX_FILE))
    {
      if (possible_types & load_path::OCT_FILE)
	{
	  fname += ".oct";
	  retval = true;
	}
      else if (possible_types & load_path::MEX_FILE)
	{
	  fname += ".mex";
	  retval = true;
	}
      else if (possible_types & load_path::M_FILE)
	{
	  fname += ".m";
	  retval = true;
	}
    }
  else
    error ("%s: %s: invalid type code = %d", who, fcn.c_str (), type);

  return retval;
} 

std::string
load_path::do_find_fcn (const std::string& fcn, std::string& dir_name,
			int type) const
{
  std::string retval;
  
  //  update ();

  dir_name = std::string ();

  const_fcn_map_iterator p = fcn_map.find (fcn);

  if (p != fcn_map.end ())
    {
      const file_info_list_type& file_info_list = p->second;

      for (const_file_info_list_iterator i = file_info_list.begin ();
	   i != file_info_list.end ();
	   i++)
	{
	  const file_info& fi = *i;

	  retval = file_ops::concat (fi.dir_name, fcn);

	  if (check_file_type (retval, type, fi.types,
			       fcn, "load_path::do_find_fcn"))
	    {
	      dir_name = fi.dir_name;
	      break;
	    }
	  else
	    retval = std::string ();
	}
    }

  return retval;
}

std::string
load_path::do_find_private_fcn (const std::string& dir,
				const std::string& fcn, int type) const
{
  std::string retval;

  //  update ();

  const_private_fcn_map_iterator q = private_fcn_map.find (dir);

  if (q != private_fcn_map.end ())
    {
      const dir_info::fcn_file_map_type& m = q->second;

      dir_info::const_fcn_file_map_iterator p = m.find (fcn);

      if (p != m.end ())
	{
	  std::string fname
	    = file_ops::concat (file_ops::concat (dir, "private"), fcn);

	  if (check_file_type (fname, type, p->second, fcn,
			       "load_path::find_private_fcn"))
	    retval = fname;
	}
    }

  return retval;
}

std::string
load_path::do_find_method (const std::string& class_name,
			   const std::string& meth,
			   std::string& dir_name, int type) const
{
  std::string retval;

  //  update ();

  dir_name = std::string ();

  const_method_map_iterator q = method_map.find (class_name);

  if (q != method_map.end ())
    {
      const fcn_map_type& m = q->second;

      const_fcn_map_iterator p = m.find (meth);

      if (p != m.end ())
	{
	  const file_info_list_type& file_info_list = p->second;

	  for (const_file_info_list_iterator i = file_info_list.begin ();
	       i != file_info_list.end ();
	       i++)
	    {
	      const file_info& fi = *i;

	      retval = file_ops::concat (fi.dir_name, meth);

	      bool found = check_file_type (retval, type, fi.types,
					    meth, "load_path::do_find_method");

	      if (found)
		{
		  dir_name = fi.dir_name;
		  break;
		}
	      else
		retval = std::string ();
	    }
	}
    }

  return retval;
}

std::list<std::string>
load_path::do_methods (const std::string& class_name) const
{
  std::list<std::string> retval;

  //  update ();

  const_method_map_iterator q = method_map.find (class_name);

  if (q != method_map.end ())
    {
      const fcn_map_type& m = q->second;

      for (const_fcn_map_iterator p = m.begin (); p != m.end (); p++)
	retval.push_back (p->first);
    }

  if (! retval.empty ())
    retval.sort ();

  return retval;
}

std::string
load_path::do_find_file (const std::string& file) const
{
  std::string retval;

  if (file.find_first_of (file_ops::dir_sep_chars ()) != std::string::npos)
    {
      if (octave_env::absolute_pathname (file)
	  || octave_env::rooted_relative_pathname (file))
	{
	  file_stat fs (file);

	  if (fs.exists ())
	    return file;
	}
      else
	{
	  for (const_dir_info_list_iterator p = dir_info_list.begin ();
	       p != dir_info_list.end ();
	       p++)
	    {
	      std::string tfile = file_ops::concat (p->dir_name, file);

	      file_stat fs (tfile);

	      if (fs.exists ())
		return tfile;
	    }
	}
    }
  else
    {
      for (const_dir_info_list_iterator p = dir_info_list.begin ();
	   p != dir_info_list.end ();
	   p++)
	{
	  string_vector all_files = p->all_files;

	  octave_idx_type len = all_files.length ();

	  for (octave_idx_type i = 0; i < len; i++)
	    {
	      if (all_files[i] == file)
		return file_ops::concat (p->dir_name, file);
	    }
	}
    }

  return retval;
}

std::string
load_path::do_find_dir (const std::string& dir) const
{
  std::string retval;

  if (dir.find_first_of (file_ops::dir_sep_chars ()) != std::string::npos
      && (octave_env::absolute_pathname (dir)
	  || octave_env::rooted_relative_pathname (dir)))
    {
      file_stat fs (dir);

      if (fs.exists () && fs.is_dir ())
	return dir;
    }
  else
    {
      for (const_dir_info_list_iterator p = dir_info_list.begin ();
	   p != dir_info_list.end ();
	   p++)
	{
	  std::string dname = p->dir_name;

	  size_t dname_len = dname.length ();

	  if (dname.substr (dname_len - 1) == file_ops::dir_sep_str ())
	    dname = dname.substr (0, dname_len - 1);

	  size_t dir_len = dir.length ();

	  if (dname_len >= dir_len
	      && file_ops::is_dir_sep (dname[dname_len - dir_len - 1])
	      && dir.compare (dname.substr (dname_len - dir_len)) == 0)
	    {
	      file_stat fs (p->dir_name);

	      if (fs.exists () && fs.is_dir ())
		return p->dir_name;
	    }
	}
    }

  return retval;
}

std::string
load_path::do_find_first_of (const string_vector& flist) const
{
  std::string retval;

  std::string dir_name;
  std::string file_name;

  octave_idx_type flen = flist.length ();
  octave_idx_type rel_flen = 0;

  string_vector rel_flist (flen);

  for (octave_idx_type i = 0; i < flen; i++)
    {
      if (octave_env::absolute_pathname (flist[i]))
	{
	  file_stat fs (flist[i]);

	  if (fs.exists ())
	    return flist[i];
	}
      else
	rel_flist[rel_flen++] = flist[i];
    }

  rel_flist.resize (rel_flen);

  for (const_dir_info_list_iterator p = dir_info_list.begin ();
       p != dir_info_list.end ();
       p++)
    {
      string_vector all_files = p->all_files;

      octave_idx_type len = all_files.length ();

      for (octave_idx_type i = 0; i < len; i++)
	{
	  for (octave_idx_type j = 0; j < rel_flen; j++)
	    {
	      if (all_files[i] == rel_flist[j])
		{
		  dir_name = p->dir_name;
		  file_name = rel_flist[j];

		  goto done;
		}
	    }
	}
    }

 done:

  if (! dir_name.empty ())
    retval = file_ops::concat (dir_name, file_name);

  return retval;
}

string_vector
load_path::do_find_all_first_of (const string_vector& flist) const
{
  std::list<std::string> retlist;

  std::string dir_name;
  std::string file_name;

  octave_idx_type flen = flist.length ();
  octave_idx_type rel_flen = 0;

  string_vector rel_flist (flen);

  for (octave_idx_type i = 0; i < flen; i++)
    {
      if (octave_env::absolute_pathname (flist[i]))
	{
	  file_stat fs (flist[i]);

	  if (fs.exists ())
	    retlist.push_back (flist[i]);
	}
      else
	rel_flist[rel_flen++] = flist[i];
    }

  rel_flist.resize (rel_flen);

  for (const_dir_info_list_iterator p = dir_info_list.begin ();
       p != dir_info_list.end ();
       p++)
    {
      string_vector all_files = p->all_files;

      octave_idx_type len = all_files.length ();

      for (octave_idx_type i = 0; i < len; i++)
	{
	  for (octave_idx_type j = 0; j < rel_flen; j++)
	    {
	      if (all_files[i] == rel_flist[j])
		retlist.push_back
		  (file_ops::concat (p->dir_name, rel_flist[j]));
	    }
	}
    }

  size_t retsize = retlist.size ();

  string_vector retval (retsize);

  for (size_t i = 0; i < retsize; i++)
    {
      retval[i] = retlist.front ();

      retlist.pop_front ();
    }

  return retval;
}

string_vector
load_path::do_dirs (void) const
{
  size_t len = dir_info_list.size ();

  string_vector retval (len);

  octave_idx_type k = 0;

  for (const_dir_info_list_iterator i = dir_info_list.begin ();
       i != dir_info_list.end ();
       i++)
    retval[k++] = i->dir_name;

  return retval;
}

std::list<std::string>
load_path::do_dir_list (void) const
{
  std::list<std::string> retval;

  for (const_dir_info_list_iterator i = dir_info_list.begin ();
       i != dir_info_list.end ();
       i++)
    retval.push_back (i->dir_name);

  return retval;
}

string_vector
load_path::do_files (const std::string& dir) const
{
  string_vector retval;

  const_dir_info_list_iterator i = find_dir_info (dir);

  if (i != dir_info_list.end ())
    retval = i->fcn_files;

  return retval;
}

string_vector
load_path::do_fcn_names (void) const
{
  size_t len = fcn_map.size ();

  string_vector retval (len);

  octave_idx_type count = 0;

  for (const_fcn_map_iterator p = fcn_map.begin ();
       p != fcn_map.end ();
       p++)
    retval[count++] = p->first;

  return retval;
}

std::string
load_path::do_path (void) const
{
  std::string xpath;

  string_vector xdirs = load_path::dirs ();

  octave_idx_type len = xdirs.length ();

  if (len > 0)
    xpath = xdirs[0];

  for (octave_idx_type i = 1; i < len; i++)
    xpath += dir_path::path_sep_str () + xdirs[i];

  return xpath;
}

void
print_types (std::ostream& os, int types)
{
  bool printed_type = false;

  if (types & load_path::OCT_FILE)
    {
      os << "oct";
      printed_type = true;
    }

  if (types & load_path::MEX_FILE)
    {
      if (printed_type)
	os << "|";
      os << "mex";
      printed_type = true;
    }

  if (types & load_path::M_FILE)
    {
      if (printed_type)
	os << "|";
      os << "m";
      printed_type = true;
    }
}

void
print_fcn_list (std::ostream& os,
		const load_path::dir_info::fcn_file_map_type& lst)
{
  for (load_path::dir_info::const_fcn_file_map_iterator p = lst.begin ();
       p != lst.end ();
       p++)
    {
      os << "  " << p->first << " (";

      print_types (os, p->second);

      os << ")\n";
    }
}

string_vector
get_file_list (const load_path::dir_info::fcn_file_map_type& lst)
{
  octave_idx_type n = lst.size ();

  string_vector retval (n);

  octave_idx_type count = 0;

  for (load_path::dir_info::const_fcn_file_map_iterator p = lst.begin ();
       p != lst.end ();
       p++)
    {
      std::string nm = p->first;

      int types = p->second;

      if (types & load_path::OCT_FILE)
	nm += ".oct";
      else if (types & load_path::MEX_FILE)
	nm += ".mex";
      else
	nm += ".m";

      retval[count++] = nm;
    }

  return retval;
}

void
load_path::do_display (std::ostream& os) const
{
  for (const_dir_info_list_iterator i = dir_info_list.begin ();
       i != dir_info_list.end ();
       i++)
    {
      string_vector fcn_files = i->fcn_files;

      if (! fcn_files.empty ())
	{
	  os << "\n*** function files in " << i->dir_name << ":\n\n";

	  fcn_files.list_in_columns (os);
	}

      const dir_info::method_file_map_type& method_file_map
	= i->method_file_map;

      if (! method_file_map.empty ())
	{
	  for (dir_info::const_method_file_map_iterator p = method_file_map.begin ();
	       p != method_file_map.end ();
	       p++)
	    {
	      os << "\n*** methods in " << i->dir_name
		 << "/@" << p->first << ":\n\n";

	      const dir_info::class_info& ci = p->second;

	      string_vector method_files = get_file_list (ci.method_file_map);

	      method_files.list_in_columns (os);
	    }
	}
    }

  for (const_private_fcn_map_iterator i = private_fcn_map.begin ();
       i != private_fcn_map.end (); i++)
    {
      os << "\n*** private functions in "
	 << file_ops::concat (i->first, "private") << ":\n\n";

      print_fcn_list (os, i->second);
    }

#if defined (DEBUG_LOAD_PATH)

  for (const_fcn_map_iterator i = fcn_map.begin ();
       i != fcn_map.end ();
       i++)
    {
      os << i->first << ":\n";

      const file_info_list_type& file_info_list = i->second;

      for (const_file_info_list_iterator p = file_info_list.begin ();
	   p != file_info_list.end ();
	   p++)
	{
	  os << "  " << p->dir_name << " (";

	  print_types (os, p->types);

	  os << ")\n";
	}
    }

  for (const_method_map_iterator i = method_map.begin ();
       i != method_map.end ();
       i++)
    {
      os << "CLASS " << i->first << ":\n";

      const fcn_map_type& fm = i->second;

      for (const_fcn_map_iterator q = fm.begin ();
	   q != fm.end ();
	   q++)
	{
	  os << "  " << q->first << ":\n";

	  const file_info_list_type& file_info_list = q->second;

	  for (const_file_info_list_iterator p = file_info_list.begin ();
	       p != file_info_list.end ();
	       p++)
	    {
	      os << "  " << p->dir_name << " (";

	      print_types (os, p->types);

	      os << ")\n";
	    }
	}
    }

  os << "\n";

#endif
}

void
load_path::add_to_fcn_map (const dir_info& di, bool at_end) const
{
  std::string dir_name = di.dir_name;

  string_vector fcn_files = di.fcn_files;

  octave_idx_type len = fcn_files.length ();

  for (octave_idx_type i = 0; i < len; i++)
    {
      std::string fname = fcn_files[i];

      std::string ext;
      std::string base = fname;

      size_t pos = fname.rfind ('.');

      if (pos != std::string::npos)
	{
	  base = fname.substr (0, pos);
	  ext = fname.substr (pos);
	}

      file_info_list_type& file_info_list = fcn_map[base];

      file_info_list_iterator p = file_info_list.begin ();

      while (p != file_info_list.end ())
	{
	  if (p->dir_name == dir_name)
	    break;

	  p++;
	}

      int t = 0;
      if (ext == ".m")
	t = load_path::M_FILE;
      else if (ext == ".oct")
	t = load_path::OCT_FILE;
      else if (ext == ".mex")
	t = load_path::MEX_FILE;

      if (p == file_info_list.end ())
	{
	  file_info fi (dir_name, t);

	  if (at_end)
	    file_info_list.push_back (fi);
	  else
	    file_info_list.push_front (fi);
	}
      else
	{
	  file_info& fi = *p;

	  fi.types |= t;
	}
    }
}

void
load_path::add_to_private_fcn_map (const dir_info& di) const
{
  dir_info::fcn_file_map_type private_file_map = di.private_file_map;

  if (! private_file_map.empty ())
    private_fcn_map[di.dir_name] = private_file_map;
}

void
load_path::add_to_method_map (const dir_info& di, bool at_end) const
{
  std::string dir_name = di.dir_name;

  // <CLASS_NAME, CLASS_INFO>
  dir_info::method_file_map_type method_file_map = di.method_file_map;

  for (dir_info::const_method_file_map_iterator q = method_file_map.begin ();
       q != method_file_map.end ();
       q++)
    {
      std::string class_name = q->first;

      fcn_map_type& fm = method_map[class_name];

      std::string full_dir_name
	= file_ops::concat (dir_name, "@" + class_name);

      const dir_info::class_info& ci = q->second;

      // <FCN_NAME, TYPES>
      const dir_info::fcn_file_map_type& m = ci.method_file_map;

      for (dir_info::const_fcn_file_map_iterator p = m.begin ();
	   p != m.end ();
	   p++)
	{
	  std::string base = p->first;

	  int types = p->second;

	  file_info_list_type& file_info_list = fm[base];

	  file_info_list_iterator p2 = file_info_list.begin ();

	  while (p2 != file_info_list.end ())
	    {
	      if (p2->dir_name == full_dir_name)
		break;

	      p2++;
	    }

	  if (p2 == file_info_list.end ())
	    {
	      file_info fi (full_dir_name, types);

	      if (at_end)
		file_info_list.push_back (fi);
	      else
		file_info_list.push_front (fi);
	    }
	  else
	    {
	      // FIXME -- is this possible?

	      file_info& fi = *p2;

	      fi.types = types;
	    }
	}

      // <FCN_NAME, TYPES>
      dir_info::fcn_file_map_type private_file_map = ci.private_file_map;

      if (! private_file_map.empty ())
	private_fcn_map[full_dir_name] = private_file_map;
    }
}

std::string
genpath (const std::string& dirname, const string_vector& skip)
{
  std::string retval;

  dir_entry dir (dirname);

  if (dir)
    {
      retval = dirname;

      string_vector dirlist = dir.read ();
      
      octave_idx_type len = dirlist.length ();

      for (octave_idx_type i = 0; i < len; i++)
	{
	  std::string elt = dirlist[i];

	  // FIXME -- the caller should be able to specify the list of
	  // directories to skip in addition to ".", "..", and
	  // directories beginning with "@".

	  bool skip_p = (elt == "." || elt == ".." || elt[0] == '@');

	  if (! skip_p)
	    {
	      for (octave_idx_type j = 0; j < skip.length (); j++)
		{
		  skip_p = (elt == skip[j]);
		  if (skip_p)
		    break;
		}

	      if (! skip_p)
		{
		  std::string nm = file_ops::concat (dirname, elt);

		  file_stat fs (nm);

		  if (fs && fs.is_dir ())
		    retval += dir_path::path_sep_str () + genpath (nm);
		}
	    }
	}
    }

  return retval;
}

static void
execute_pkg_add_or_del (const std::string& dir,
			const std::string& script_file)
{
  if (! octave_interpreter_ready)
    return;

  unwind_protect::begin_frame ("execute_pkg_add_or_del");

  unwind_protect_bool (input_from_startup_file);

  input_from_startup_file = true;

  std::string file = file_ops::concat (dir, script_file);

  file_stat fs (file);

  if (fs.exists ())
    source_file (file, "base");

  unwind_protect::run_frame ("execute_pkg_add_or_del");
}

void
execute_pkg_add (const std::string& dir)
{
  execute_pkg_add_or_del (dir, "PKG_ADD");
}

void
execute_pkg_del (const std::string& dir)
{
  execute_pkg_add_or_del (dir, "PKG_DEL");
}

DEFUN (genpath, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} genpath (@var{dir})\n\
Return a path constructed from @var{dir} and all its subdirectories.\n\
@end deftypefn")
{
  octave_value retval;

  if (args.length () == 1)
    {
      std::string dirname = args(0).string_value ();

      if (! error_state)
	retval = genpath (dirname);
      else
	error ("genpath: expecting argument to be a character string");
    }
  else
    print_usage ();

  return retval;
}

DEFUN (rehash, , ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} rehash ()\n\
Reinitialize Octave's load path directory cache.\n\
@end deftypefn")
{
  octave_value_list retval;

  load_path::update ();

  // FIXME -- maybe we should rename this variable since it is being
  // used for more than keeping track of the prompt time.

  // This will force updated functions to be found.
  Vlast_prompt_time.stamp ();

  return retval;
}

DEFUN (restoredefaultpath, , ,
    "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} restoredefaultpath (@dots{})\n\
Restore Octave's path to it's initial state at startup.\n\
\n\
@seealso{path, addpath, rmpath, genpath, pathdef, savepath, pathsep}\n\
@end deftypefn")
{
  load_path::initialize (true);

  return octave_value (load_path::system_path ());
}

DEFUN (__pathorig__, , ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {@var{val} =} __pathorig__ ()\n\
Return Octave's original default list of directories in which to search\n\
for function files. This corresponds to the path that exists prior to\n\
running the system's @file{octaverc}, or the users' @file{~/.octaverc}\n\
@seealso{path, addpath, rmpath, genpath, savepath, pathsep, \n\
restoredefaultpath}\n\
@end deftypefn")
{
  return octave_value (load_path::system_path ());
}

DEFUN (path, args, nargout,
    "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} path (@dots{})\n\
Modify or display Octave's load path.\n\
\n\
If @var{nargin} and @var{nargout} are zero, display the elements of\n\
Octave's load path in an easy to read format.\n\
\n\
If @var{nargin} is zero and nargout is greater than zero, return the\n\
current load path.\n\
\n\
If @var{nargin} is greater than zero, concatenate the arguments,\n\
separating them with @code{pathsep()}.  Set the internal search path\n\
to the result and return it.\n\
\n\
No checks are made for duplicate elements.\n\
@seealso{addpath, rmpath, genpath, pathdef, savepath, pathsep}\n\
@end deftypefn")
{
  octave_value retval;

  int argc = args.length () + 1;

  string_vector argv = args.make_argv ("path");

  if (! error_state)
    {
      if (argc > 1)
	{
	  std::string path = argv[1];

	  for (int i = 2; i < argc; i++)
	    path += dir_path::path_sep_str () + argv[i];

	  load_path::set (path, true);
	}

      if (nargout > 0)
	retval = load_path::path ();
      else if (argc == 1 && nargout == 0)
	{
	  octave_stdout << "\nOctave's search path contains the following directories:\n\n";

	  string_vector dirs = load_path::dirs ();

	  dirs.list_in_columns (octave_stdout);

	  octave_stdout << "\n";
	}
    }

  return retval;
}

DEFCMD (addpath, args, nargout,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} addpath (@var{dir1}, @dots{})\n\
@deftypefnx {Built-in Function} {} addpath (@var{dir1}, @dots{}, @var{option})\n\
Add @var{dir1}, @dots{} to the current function search path.  If\n\
@var{option} is @samp{\"-begin\"} or 0 (the default), prepend the\n\
directory name to the current path.  If @var{option} is @samp{\"-end\"}\n\
or 1, append the directory name to the current path.\n\
Directories added to the path must exist.\n\
@seealso{path, rmpath, genpath, pathdef, savepath, pathsep}\n\
@end deftypefn")
{
  octave_value retval;

  // Originally written by Bill Denney and Etienne Grossman.  Heavily
  // modified and translated to C++ by jwe.

  if (nargout > 0)
    retval = load_path::path ();

  int nargin = args.length ();

  if (nargin > 0)
    {
      bool append = false;

      octave_value option_arg = args(nargin-1);

      if (option_arg.is_string ())
	{
	  std::string option = option_arg.string_value ();

	  if (option == "-end")
	    {
	      append = true;
	      nargin--;
	    }
	  else if (option == "-begin")
	    nargin--;
	}
      else if (option_arg.is_numeric_type ())
	{
	  int val = option_arg.int_value ();

	  if (! error_state)
	    {
	      if (val == 0)
		append = false;
	      else if (val == 1)
		append = true;
	      else
		{
		  error ("addpath: expecting final argument to be 1 or 0");
		  return retval;
		}
	    }
	  else
	    {
	      error ("addpath: expecting final argument to be 1 or 0");
	      return retval;
	    }
	}

      for (int i = 0; i < nargin; i++)
	{
	  std::string arg = args(i).string_value ();

	  if (! error_state)
	    {
	      std::list<std::string> dir_elts = split_path (arg);

	      for (std::list<std::string>::const_iterator p = dir_elts.begin ();
		   p != dir_elts.end ();
		   p++)
		{
		  std::string dir = *p;

		  //dir = regexprep (dir_elts{j}, "//+", "/");
		  //dir = regexprep (dir, "/$", "");

		  if (append)
		    load_path::append (dir, true);
		  else
		    load_path::prepend (dir, true);
		}
	    }
	  else
	    error ("addpath: expecting all args to be character strings");
	}
    }
  else
    print_usage ();

  return retval;
}

DEFCMD (rmpath, args, nargout,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} rmpath (@var{dir1}, @dots{})\n\
Remove @var{dir1}, @dots{} from the current function search path.\n\
\n\
@seealso{path, addpath, genpath, pathdef, savepath, pathsep}\n\
@end deftypefn")
{
  // Originally by Etienne Grossmann. Heavily modified and translated
  // to C++ by jwe.

  octave_value retval;

  if (nargout > 0)
    retval = load_path::path ();

  int nargin = args.length ();

  if (nargin > 0)
    {
      for (int i = 0; i < nargin; i++)
	{
	  std::string arg = args(i).string_value ();

	  if (! error_state)
	    {
	      std::list<std::string> dir_elts = split_path (arg);

	      for (std::list<std::string>::const_iterator p = dir_elts.begin ();
		   p != dir_elts.end ();
		   p++)
		{
		  std::string dir = *p;

		  //dir = regexprep (dir_elts{j}, "//+", "/");
		  //dir = regexprep (dir, "/$", "");

		  if (! load_path::remove (dir))
		    warning ("rmpath: %s: not found", dir.c_str ());
		}
	    }
	  else
	    error ("addpath: expecting all args to be character strings");
	}
    }
  else
    print_usage ();

  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
