/*

Copyright (C) 2004 John W. Eaton

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

#if !defined (octave_base_int_matrix_h)
#define octave_base_int_matrix_h 1

#if defined (__GNUG__) && defined (USE_PRAGMA_INTERFACE_IMPLEMENTATION)
#pragma interface
#endif

#include <cstdlib>

#include <iostream>
#include <string>

#include "mx-base.h"
#include "oct-alloc.h"
#include "so-array.h"
#include "str-vec.h"

#include "error.h"
#include "ov-base.h"
#include "ov-base-mat.h"
#include "ov-base-scalar.h"
#include "ov-typeinfo.h"

// base int matrix values.

template <class T>
class
octave_base_int_matrix : public octave_base_matrix<T>
{
public:

  octave_base_int_matrix (void) : octave_base_matrix<T> () { }

  octave_base_int_matrix (const T& nda) : octave_base_matrix<T> (nda) { }

  ~octave_base_int_matrix (void) { }

  octave_value *clone (void) const { return new octave_base_int_matrix (*this); }
  octave_value *empty_clone (void) const { return new octave_base_int_matrix (); }

  octave_value *try_narrowing_conversion (void);

  bool is_real_type (void) const { return true; }

  //  void increment (void) { matrix += 1; }

  //  void decrement (void) { matrix -= 1; }

  void print_raw (std::ostream& os, bool pr_as_read_syntax = false) const;

  bool save_ascii (std::ostream& os, bool& infnan_warned,
		   bool strip_nan_and_inf);

  bool load_ascii (std::istream& is);

  bool save_binary (std::ostream& os, bool& );

  bool load_binary (std::istream& is, bool swap, 
		    oct_mach_info::float_format );

#if defined (HAVE_HDF5)
  bool save_hdf5 (hid_t loc_id, const char *name, bool);

  bool load_hdf5 (hid_t loc_id, const char *name, bool have_h5giterate_bug);
#endif
};

// base int scalar values.

template <class T>
class
octave_base_int_scalar : public octave_base_scalar<T>
{
public:

  octave_base_int_scalar (void) : octave_base_scalar<T> () { }

  octave_base_int_scalar (const T& s) : octave_base_scalar<T> (s) { }

  ~octave_base_int_scalar (void) { }

  octave_value *clone (void) const { return new octave_base_int_scalar (*this); }
  octave_value *empty_clone (void) const { return new octave_base_int_scalar (); }

  octave_value *try_narrowing_conversion (void) { return 0; }

  bool is_real_type (void) const { return true; }

  //  void increment (void) { scalar += 1; }

  //  void decrement (void) { scalar -= 1; }

  bool save_ascii (std::ostream& os, bool&, bool );

  bool load_ascii (std::istream& is);

  bool save_binary (std::ostream& os, bool& );

  bool load_binary (std::istream& is, bool swap, 
		    oct_mach_info::float_format );

#if defined (HAVE_HDF5)
  bool save_hdf5 (hid_t loc_id, const char *name, bool );

  bool load_hdf5 (hid_t loc_id, const char *name, bool have_h5giterate_bug);
#endif
};

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
