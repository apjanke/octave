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

#if !defined (octave_matrix_h)
#define octave_matrix_h 1

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
#include "ov-base-mat.h"
#include "ov-typeinfo.h"

class Octave_map;
class octave_value_list;

class tree_walker;

// Real matrix values.

class
octave_matrix : public octave_base_matrix<Matrix>
{
public:

  octave_matrix (void)
    : octave_base_matrix<Matrix> () { }

  octave_matrix (const Matrix& m)
    : octave_base_matrix<Matrix> (m) { }

  octave_matrix (const DiagMatrix& d)
    : octave_base_matrix<Matrix> (d) { }

  octave_matrix (const RowVector& v, int pcv = -1);

  octave_matrix (const ColumnVector& v, int pcv = -1);

  octave_matrix (const octave_matrix& m)
    : octave_base_matrix<Matrix> (m) { }

  ~octave_matrix (void) { }

  octave_value *clone (void) { return new octave_matrix (*this); }

  octave_value *try_narrowing_conversion (void);

  octave_value do_index_op (const octave_value_list& idx);

  void assign (const octave_value_list& idx, const Matrix& rhs);

  void assign_struct_elt (assign_op, const string& elt_nm,
			  const octave_value& rhs);

  void assign_struct_elt (assign_op, const string& elt_nm,
			  const octave_value_list& idx,
			  const octave_value& rhs);

  idx_vector index_vector (void) const { return idx_vector (matrix); }

  octave_value
  do_struct_elt_index_op (const string& nm, const octave_value_list& idx,
			  bool silent);

  octave_value do_struct_elt_index_op (const string& nm, bool silent);

  octave_lvalue struct_elt_ref (octave_value *parent, const string& nm);

  bool is_real_matrix (void) const { return true; }

  octave_value all (void) const { return matrix.all (); }
  octave_value any (void) const { return matrix.any (); }

  bool is_real_type (void) const { return true; }

  bool is_matrix_type (void) const { return true; }

  bool is_numeric_type (void) const { return true; }

  bool valid_as_scalar_index (void) const;

  bool valid_as_zero_index (void) const { return is_zero_by_zero (); }

  bool is_true (void) const;

  double double_value (bool = false) const;

  double scalar_value (bool frc_str_conv = false) const
    { return double_value (frc_str_conv); }

  Matrix matrix_value (bool = false) const { return matrix; }

  Complex complex_value (bool = false) const;

  ComplexMatrix complex_matrix_value (bool = false) const
    { return matrix; }

  void increment (void) { matrix += 1.0; }

  void decrement (void) { matrix -= 1.0; }

  octave_value convert_to_str (void) const;

private:

  DECLARE_OCTAVE_ALLOCATOR

  DECLARE_OV_TYPEID_FUNCTIONS_AND_DATA
};

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
