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

#if !defined (octave_complex_h)
#define octave_complex_h 1

#if defined (__GNUG__)
#pragma interface
#endif

#include <cstdlib>

#include <string>

class ostream;

#include "mx-base.h"
#include "oct-alloc.h"
#include "str-vec.h"

#include "error.h"
#include "ov-base.h"
#include "ov-typeinfo.h"

class Octave_map;
class octave_value_list;

class tree_walker;

// Complex scalar values.

class
octave_complex : public octave_base_value
{
public:

  octave_complex (void)
    : octave_base_value () { }

  octave_complex (const Complex& c)
    : octave_base_value (), scalar (c) { }

  octave_complex (const octave_complex& c)
    : octave_base_value (), scalar (c.scalar) { }

  ~octave_complex (void) { }

  octave_value *clone (void) { return new octave_complex (*this); }

  octave_value *try_narrowing_conversion (void);

  octave_value do_index_op (const octave_value_list& idx);

  int rows (void) const { return 1; }
  int columns (void) const { return 1; }

  int length (void) const
  {
    int r = rows ();
    int c = columns ();

    return r > c ? r : c;
  }

  bool is_defined (void) const { return true; }

  bool is_constant (void) const { return true; }

  bool is_complex_scalar (void) const { return true; }

  octave_value all (void) const { return (scalar != 0.0); }
  octave_value any (void) const { return (scalar != 0.0); }

  bool is_complex_type (void) const { return true; }

  bool is_scalar_type (void) const { return true; }

  bool is_numeric_type (void) const { return true; }

  // XXX FIXME XXX ???
  bool valid_as_scalar_index (void) const { return false; }
  bool valid_as_zero_index (void) const { return false; }

  bool is_true (void) const { return (scalar != 0.0); }

  bool is_empty (void) const { return (rows () == 0 && columns () == 0); }

  double double_value (bool = false) const;

  double scalar_value (bool frc_str_conv = false) const
    { return double_value (frc_str_conv); }

  Matrix matrix_value (bool = false) const;

  Complex complex_value (bool = false) const;

  ComplexMatrix complex_matrix_value (bool = false) const;

  void increment (void) { scalar += 1.0; }

  void decrement (void) { scalar -= 1.0; }

  void print (ostream& os, bool pr_as_read_syntax = false) const;

  void print_raw (ostream& os, bool pr_as_read_syntax = false) const;

  bool print_name_tag (ostream& os, const string& name) const;

private:

  Complex scalar;

  DECLARE_OV_TYPEID_FUNCTIONS_AND_DATA

  DECLARE_OCTAVE_ALLOCATOR
};

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
