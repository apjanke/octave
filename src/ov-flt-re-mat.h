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

#if !defined (octave_float_matrix_h)
#define octave_float_matrix_h 1

#include <cstdlib>

#include <iostream>
#include <string>

#include "mx-base.h"
#include "oct-alloc.h"
#include "str-vec.h"

#include "error.h"
#include "oct-stream.h"
#include "ov-base.h"
#include "ov-base-mat.h"
#include "ov-typeinfo.h"

#include "MatrixType.h"

class Octave_map;
class octave_value_list;

class tree_walker;

// Real matrix values.

class
OCTINTERP_API
octave_float_matrix : public octave_base_matrix<FloatNDArray>
{
public:

  octave_float_matrix (void)
    : octave_base_matrix<FloatNDArray> () { }

  octave_float_matrix (const FloatMatrix& m)
    : octave_base_matrix<FloatNDArray> (m) { }

  octave_float_matrix (const FloatMatrix& m, const MatrixType& t)
    : octave_base_matrix<FloatNDArray> (m, t) { }

  octave_float_matrix (const FloatNDArray& nda)
    : octave_base_matrix<FloatNDArray> (nda) { }

  octave_float_matrix (const ArrayN<float>& m)
    : octave_base_matrix<FloatNDArray> (FloatNDArray (m)) { }

  octave_float_matrix (const FloatDiagMatrix& d)
    : octave_base_matrix<FloatNDArray> (FloatMatrix (d)) { }

  octave_float_matrix (const FloatRowVector& v)
    : octave_base_matrix<FloatNDArray> (FloatMatrix (v)) { }

  octave_float_matrix (const FloatColumnVector& v)
    : octave_base_matrix<FloatNDArray> (FloatMatrix (v)) { }

  octave_float_matrix (const octave_float_matrix& m)
    : octave_base_matrix<FloatNDArray> (m) { }

  ~octave_float_matrix (void) { }

  octave_base_value *clone (void) const { return new octave_float_matrix (*this); }
  octave_base_value *empty_clone (void) const { return new octave_float_matrix (); }

  octave_base_value *try_narrowing_conversion (void);

  idx_vector index_vector (void) const { return idx_vector (matrix); }

  bool is_real_matrix (void) const { return true; }

  bool is_real_type (void) const { return true; }

  bool is_single_type (void) const { return true; }

  bool is_float_type (void) const { return true; }

  int8NDArray
  int8_array_value (void) const { return int8NDArray (matrix); }

  int16NDArray
  int16_array_value (void) const { return int16NDArray (matrix); }

  int32NDArray
  int32_array_value (void) const { return int32NDArray (matrix); }

  int64NDArray
  int64_array_value (void) const { return int64NDArray (matrix); }

  uint8NDArray
  uint8_array_value (void) const { return uint8NDArray (matrix); }

  uint16NDArray
  uint16_array_value (void) const { return uint16NDArray (matrix); }

  uint32NDArray
  uint32_array_value (void) const { return uint32NDArray (matrix); }

  uint64NDArray
  uint64_array_value (void) const { return uint64NDArray (matrix); }

  double double_value (bool = false) const;

  float float_value (bool = false) const;

  double scalar_value (bool frc_str_conv = false) const
    { return double_value (frc_str_conv); }

  float float_scalar_value (bool frc_str_conv = false) const
    { return float_value (frc_str_conv); }

  Matrix matrix_value (bool = false) const;

  FloatMatrix float_matrix_value (bool = false) const;

  Complex complex_value (bool = false) const;

  FloatComplex float_complex_value (bool = false) const;

  ComplexMatrix complex_matrix_value (bool = false) const;

  FloatComplexMatrix float_complex_matrix_value (bool = false) const;

  ComplexNDArray complex_array_value (bool = false) const;
   
  FloatComplexNDArray float_complex_array_value (bool = false) const;

  boolNDArray bool_array_value (bool warn = false) const;

  charNDArray char_array_value (bool = false) const;
  
  NDArray array_value (bool = false) const;
  
  FloatNDArray float_array_value (bool = false) const { return matrix; }

  SparseMatrix sparse_matrix_value (bool = false) const;

  SparseComplexMatrix sparse_complex_matrix_value (bool = false) const;

  octave_value diag (octave_idx_type k = 0) const;

  void increment (void) { matrix += 1.0; }

  void decrement (void) { matrix -= 1.0; }

  octave_value convert_to_str_internal (bool pad, bool force, char type) const;

  void print_raw (std::ostream& os, bool pr_as_read_syntax = false) const;

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
    { return os.write (matrix, block_size, output_type, skip, flt_fmt); }

  mxArray *as_mxArray (void) const;

  octave_value erf (void) const;
  octave_value erfc (void) const;
  octave_value gamma (void) const;
  octave_value lgamma (void) const;
  octave_value abs (void) const;
  octave_value acos (void) const;
  octave_value acosh (void) const;
  octave_value angle (void) const;
  octave_value arg (void) const;
  octave_value asin (void) const;
  octave_value asinh (void) const;
  octave_value atan (void) const;
  octave_value atanh (void) const;
  octave_value ceil (void) const;
  octave_value conj (void) const;
  octave_value cos (void) const;
  octave_value cosh (void) const;
  octave_value exp (void) const;
  octave_value expm1 (void) const;
  octave_value fix (void) const;
  octave_value floor (void) const;
  octave_value imag (void) const;
  octave_value log (void) const;
  octave_value log2 (void) const;
  octave_value log10 (void) const;
  octave_value log1p (void) const;
  octave_value real (void) const;
  octave_value round (void) const;
  octave_value roundb (void) const;
  octave_value signum (void) const;
  octave_value sin (void) const;
  octave_value sinh (void) const;
  octave_value sqrt (void) const;
  octave_value tan (void) const;
  octave_value tanh (void) const;
  octave_value finite (void) const;
  octave_value isinf (void) const;
  octave_value isna (void) const;
  octave_value isnan (void) const;

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
