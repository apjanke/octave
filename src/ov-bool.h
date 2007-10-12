/*

Copyright (C) 1996, 1997, 1998, 2000, 2002, 2003, 2004, 2005, 2006,
              2007 John W. Eaton

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

#if !defined (octave_bool_h)
#define octave_bool_h 1

#include <cstdlib>

#include <iostream>
#include <string>

#include "lo-utils.h"
#include "mx-base.h"
#include "oct-alloc.h"
#include "str-vec.h"

#include "oct-stream.h"
#include "ov-base.h"
#include "ov-base-scalar.h"
#include "ov-typeinfo.h"

class Octave_map;
class octave_value_list;

class tree_walker;

// Real scalar values.

class
octave_bool : public octave_base_scalar<bool>
{
public:

  octave_bool (void)
    : octave_base_scalar<bool> (false) { }

  octave_bool (bool b)
    : octave_base_scalar<bool> (b) { }

  octave_bool (const octave_bool& s)
    : octave_base_scalar<bool> (s) { }

  ~octave_bool (void) { }

  octave_base_value *clone (void) const { return new octave_bool (*this); }
  octave_base_value *empty_clone (void) const { return new octave_bool (); }

  type_conv_fcn numeric_conversion_function (void) const;

  octave_value do_index_op (const octave_value_list& idx,
			    bool resize_ok = false);

  idx_vector index_vector (void) const { return idx_vector (scalar); }

  bool is_real_scalar (void) const { return true; }

  bool is_bool_scalar (void) const { return true; }

  bool is_bool_type (void) const { return true; }

  bool is_real_type (void) const { return true; }

  bool valid_as_scalar_index (void) const { return scalar; }

  bool valid_as_zero_index (void) const { return ! scalar; }

  bool is_true (void) const { return scalar; }

  int8NDArray
  int8_array_value (void) const
    { return int8NDArray (dim_vector (1, 1), scalar); }

  int16NDArray
  int16_array_value (void) const
    { return int16NDArray (dim_vector (1, 1), scalar); }

  int32NDArray
  int32_array_value (void) const
    { return int32NDArray (dim_vector (1, 1), scalar); }

  int64NDArray
  int64_array_value (void) const
    { return int64NDArray (dim_vector (1, 1), scalar); }

  uint8NDArray
  uint8_array_value (void) const
    { return uint8NDArray (dim_vector (1, 1), scalar); }

  uint16NDArray
  uint16_array_value (void) const
    { return uint16NDArray (dim_vector (1, 1), scalar); }

  uint32NDArray
  uint32_array_value (void) const
    { return uint32NDArray (dim_vector (1, 1), scalar); }

  uint64NDArray
  uint64_array_value (void) const
    { return uint64NDArray (dim_vector (1, 1), scalar); }

  double double_value (bool = false) const { return scalar; }

  double scalar_value (bool = false) const { return scalar; }

  Matrix matrix_value (bool = false) const
    { return Matrix (1, 1, scalar); }

  NDArray array_value (bool = false) const
    { return NDArray (dim_vector (1, 1), scalar); }

  Complex complex_value (bool = false) const { return scalar; }

  ComplexMatrix complex_matrix_value (bool = false) const
    { return  ComplexMatrix (1, 1, Complex (scalar)); }

  ComplexNDArray complex_array_value (bool = false) const
    { return ComplexNDArray (dim_vector (1, 1), Complex (scalar)); }

  SparseMatrix sparse_matrix_value (bool = false) const
    { return SparseMatrix (Matrix (1, 1, scalar)); }

  // XXX FIXME XXX Need SparseComplexMatrix (Matrix) constructor!!!
  SparseComplexMatrix sparse_complex_matrix_value (bool = false) const
    { return SparseComplexMatrix (sparse_matrix_value ()); }

  SparseBoolMatrix sparse_bool_matrix_value (bool = false) const
    { return SparseBoolMatrix (boolMatrix (1, 1, scalar)); }

  charNDArray
  char_array_value (bool = false) const
  {
    charNDArray retval (dim_vector (1, 1));
    retval(0) = static_cast<char> (scalar);
    return retval;
  }

  bool bool_value (bool = false) const { return scalar; }

  boolMatrix bool_matrix_value (bool = false) const
    { return boolMatrix (1, 1, scalar); }

  boolNDArray bool_array_value (bool = false) const
    { return boolNDArray (dim_vector (1, 1), scalar); }

  octave_value resize (const dim_vector& dv, bool fill = false) const;

  octave_value convert_to_str_internal (bool pad, bool force, char type) const;

  bool save_ascii (std::ostream& os);

  bool load_ascii (std::istream& is);

  bool save_binary (std::ostream& os, bool& save_as_floats);

  bool load_binary (std::istream& is, bool swap, 
		    oct_mach_info::float_format fmt);

#if defined (HAVE_HDF5)
  bool save_hdf5 (hid_t loc_id, const char *name, bool save_as_floats);

  bool load_hdf5 (hid_t loc_id, const char *name, bool have_h5giterate_bug);
#endif

  int write (octave_stream& os, int block_size,
	     oct_data_conv::data_type output_type, int skip,
	     oct_mach_info::float_format flt_fmt) const
    {
      return os.write (bool_array_value (), block_size, output_type,
		       skip, flt_fmt);
    }

  mxArray *as_mxArray (void) const;

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
