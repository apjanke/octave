/*

Copyright (C) 1996, 1997 John W. Eaton

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

#if !defined (octave_function_h)
#define octave_function_h 1

#if defined (__GNUG__)
#pragma interface
#endif

#include <ctime>

#include <string>

#include "oct-alloc.h"
#include "ov-base.h"
#include "ov-typeinfo.h"

// Functions.

class
octave_function : public octave_base_value
{
public:

  octave_function (const octave_function& f)
    : octave_base_value (), my_name (f.my_name), doc (f.doc) { }

  ~octave_function (void) { }

  // This should only be called for derived types.

  octave_function *clone (void);

  void *operator new (size_t size)
    { return allocator.alloc (size); }

  void operator delete (void *p, size_t size)
    { allocator.free (p, size); }

  bool is_defined (void) const { return true; }

  bool is_function (void) const { return true; }

  virtual bool is_system_fcn_file (void) { return false; }

  virtual string fcn_file_name (void) const { return string (); }

  virtual time_t time_parsed (void) const { return 0; }

  string name (void) const { return my_name; }

  string doc_string (void) const { return doc; }

protected:

  octave_function (const string& nm, const string& ds)
    : my_name (nm), doc (ds) { }

private:

  octave_function (void);

  // The name of this function.
  string my_name;

  // The help text for this function.
  string doc;

  // For custom memory management.
  static octave_allocator allocator;
};

#endif

/*
;; Local Variables: ***
;; mode: C++ ***
;; End: ***
*/
