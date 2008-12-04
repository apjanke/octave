/*

Copyright (C) 2008 Jaroslav Hajek <highegg@gmail.com>

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

#include "ov-perm.h"
#include "ov-flt-perm.h"
#include "ov-re-mat.h"
#include "ov-scalar.h"
#include "error.h"
#include "gripes.h"
#include "ops.h"

octave_value
octave_perm_matrix::subsref (const std::string& type,
                             const std::list<octave_value_list>& idx)
{
  octave_value retval;

  switch (type[0])
    {
    case '(':
      retval = do_index_op (idx.front ());
      break;

    case '{':
    case '.':
      {
	std::string nm = type_name ();
	error ("%s cannot be indexed with %c", nm.c_str (), type[0]);
      }
      break;

    default:
      panic_impossible ();
    }

  return retval.next_subsref (type, idx);
}

octave_value
octave_perm_matrix::do_index_op (const octave_value_list& idx,
                                 bool resize_ok)
{
  octave_value retval;
  octave_idx_type nidx = idx.length ();
  idx_vector idx0, idx1;
  if (nidx == 2)
    {
      idx0 = idx(0).index_vector ();
      idx1 = idx(1).index_vector ();
    }

  // This hack is to allow constructing permutation matrices using
  // eye(n)(p,:), eye(n)(:,q) && eye(n)(p,q) where p & q are permutation
  // vectors. 
  // Note that, for better consistency, eye(n)(:,:) still converts to a full
  // matrix.
  if (! error_state && nidx == 2)
    {
      bool left = idx0.is_permutation (matrix.rows ());
      bool right = idx1.is_permutation (matrix.cols ());

      if (left && right)
        {
          if (idx0.is_colon ()) left = false;
          if (idx1.is_colon ()) right = false;
          if (left || right)
            {
              PermMatrix p = matrix;
              if (left)
                p = PermMatrix (idx0, false) * p;
              if (right)
                p = p * PermMatrix (idx1, true);
              retval = octave_value (p, is_single_type ());
            }
        }
    }

  // if error_state is set, we've already griped.
  if (! error_state && ! retval.is_defined ())
    {
      if (nidx == 2 && ! resize_ok &&
          idx0.is_scalar () && idx1.is_scalar ())
        {
          retval = matrix.checkelem (idx0(0), idx1(0));
        }
      else
        retval = to_dense ().do_index_op (idx, resize_ok);
    }

  return retval;
}

bool
octave_perm_matrix::is_true (void) const
{
  return to_dense ().is_true ();
}

bool
octave_perm_matrix::valid_as_scalar_index (void) const
{
  return false;
}

double
octave_perm_matrix::double_value (bool) const
{
  double retval = lo_ieee_nan_value ();

  if (numel () > 0)
    {
      gripe_implicit_conversion ("Octave:array-as-scalar",
				 type_name (), "real scalar");

      retval = matrix (0, 0);
    }
  else
    gripe_invalid_conversion (type_name (), "real scalar");

  return retval;
}

float
octave_perm_matrix::float_value (bool) const
{
  float retval = lo_ieee_float_nan_value ();

  if (numel () > 0)
    {
      gripe_implicit_conversion ("Octave:array-as-scalar",
				 type_name (), "real scalar");

      retval = matrix (0, 0);
    }
  else
    gripe_invalid_conversion (type_name (), "real scalar");

  return retval;
}

Complex
octave_perm_matrix::complex_value (bool) const
{
  double tmp = lo_ieee_nan_value ();

  Complex retval (tmp, tmp);

  if (rows () > 0 && columns () > 0)
    {
      gripe_implicit_conversion ("Octave:array-as-scalar",
				 type_name (), "complex scalar");

      retval = matrix (0, 0);
    }
  else
    gripe_invalid_conversion (type_name (), "complex scalar");

  return retval;
}

FloatComplex
octave_perm_matrix::float_complex_value (bool) const
{
  float tmp = lo_ieee_float_nan_value ();

  FloatComplex retval (tmp, tmp);

  if (rows () > 0 && columns () > 0)
    {
      gripe_implicit_conversion ("Octave:array-as-scalar",
				 type_name (), "complex scalar");

      retval = matrix (0, 0);
    }
  else
    gripe_invalid_conversion (type_name (), "complex scalar");

  return retval;
}

#define FORWARD_MATRIX_VALUE(TYPE, PREFIX) \
TYPE \
octave_perm_matrix::PREFIX ## _value (bool frc_str_conv) const \
{ \
  return to_dense ().PREFIX ## _value (frc_str_conv); \
}

FORWARD_MATRIX_VALUE (Matrix, matrix)
FORWARD_MATRIX_VALUE (FloatMatrix, float_matrix)
FORWARD_MATRIX_VALUE (ComplexMatrix, complex_matrix)
FORWARD_MATRIX_VALUE (FloatComplexMatrix, float_complex_matrix)

FORWARD_MATRIX_VALUE (NDArray, array)
FORWARD_MATRIX_VALUE (FloatNDArray, float_array)
FORWARD_MATRIX_VALUE (ComplexNDArray, complex_array)
FORWARD_MATRIX_VALUE (FloatComplexNDArray, float_complex_array)

FORWARD_MATRIX_VALUE (boolNDArray, bool_array)
FORWARD_MATRIX_VALUE (charNDArray, char_array)

FORWARD_MATRIX_VALUE (SparseMatrix, sparse_matrix)
FORWARD_MATRIX_VALUE (SparseComplexMatrix, sparse_complex_matrix)

idx_vector
octave_perm_matrix::index_vector (void) const
{
  return to_dense ().index_vector ();
}

octave_value
octave_perm_matrix::convert_to_str_internal (bool pad, bool force, char type) const
{
  return to_dense ().convert_to_str_internal (pad, force, type);
}

bool 
octave_perm_matrix::save_ascii (std::ostream& os)
{
  // FIXME: this should probably save the matrix as permutation.
  return to_dense ().save_ascii (os);
}

bool 
octave_perm_matrix::save_binary (std::ostream& os, bool& save_as_floats)
{
  return to_dense ().save_binary (os, save_as_floats);
}

#if defined (HAVE_HDF5)

bool
octave_perm_matrix::save_hdf5 (hid_t loc_id, const char *name, bool save_as_floats)
{
  return to_dense ().save_hdf5 (loc_id, name, save_as_floats);
}

#endif

void
octave_perm_matrix::print_raw (std::ostream& os,
			  bool pr_as_read_syntax) const
{
  return to_dense ().print_raw (os, pr_as_read_syntax);
}

mxArray *
octave_perm_matrix::as_mxArray (void) const
{
  return to_dense ().as_mxArray ();
}

bool
octave_perm_matrix::print_as_scalar (void) const
{
  dim_vector dv = dims ();

  return (dv.all_ones () || dv.any_zero ());
}

void
octave_perm_matrix::print (std::ostream& os, bool pr_as_read_syntax) const
{
  to_dense ().print (os, pr_as_read_syntax);
}

int
octave_perm_matrix::write (octave_stream& os, int block_size,
                                oct_data_conv::data_type output_type, int skip,
                                oct_mach_info::float_format flt_fmt) const
{ 
  return to_dense ().write (os, block_size, output_type, skip, flt_fmt); 
}

void
octave_perm_matrix::print_info (std::ostream& os,
				    const std::string& prefix) const
{
  matrix.print_info (os, prefix);
}


octave_value
octave_perm_matrix::to_dense (void) const
{
  if (! dense_cache.is_defined ())
      dense_cache = Matrix (matrix);

  return dense_cache;
}

#define FORWARD_MAPPER(MAP) \
  octave_value \
  octave_perm_matrix::MAP (void) const \
  { \
    return to_dense ().MAP (); \
  }

FORWARD_MAPPER (erf)
FORWARD_MAPPER (erfc)
FORWARD_MAPPER (gamma)
FORWARD_MAPPER (lgamma)
FORWARD_MAPPER (abs)
FORWARD_MAPPER (acos)
FORWARD_MAPPER (acosh)
FORWARD_MAPPER (angle)
FORWARD_MAPPER (arg)
FORWARD_MAPPER (asin)
FORWARD_MAPPER (asinh)
FORWARD_MAPPER (atan)
FORWARD_MAPPER (atanh)
FORWARD_MAPPER (ceil)
FORWARD_MAPPER (conj)
FORWARD_MAPPER (cos)
FORWARD_MAPPER (cosh)
FORWARD_MAPPER (exp)
FORWARD_MAPPER (expm1)
FORWARD_MAPPER (fix)
FORWARD_MAPPER (floor)
FORWARD_MAPPER (imag)
FORWARD_MAPPER (log)
FORWARD_MAPPER (log2)
FORWARD_MAPPER (log10)
FORWARD_MAPPER (log1p)
FORWARD_MAPPER (real)
FORWARD_MAPPER (round)
FORWARD_MAPPER (roundb)
FORWARD_MAPPER (signum)
FORWARD_MAPPER (sin)
FORWARD_MAPPER (sinh)
FORWARD_MAPPER (sqrt)
FORWARD_MAPPER (tan)
FORWARD_MAPPER (tanh)
FORWARD_MAPPER (finite)
FORWARD_MAPPER (isinf)
FORWARD_MAPPER (isna)
FORWARD_MAPPER (isnan)

DEFINE_OCTAVE_ALLOCATOR (octave_perm_matrix);

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA (octave_perm_matrix, 
                                     "permutation matrix", "double");

static octave_base_value *
default_numeric_conversion_function (const octave_base_value& a)
{
  CAST_CONV_ARG (const octave_perm_matrix&);

  return new octave_matrix (v.matrix_value ());
}

octave_base_value::type_conv_info
octave_perm_matrix::numeric_conversion_function (void) const
{
  return octave_base_value::type_conv_info (default_numeric_conversion_function,
                                            octave_matrix::static_type_id ());
}

static octave_base_value *
default_numeric_demotion_function (const octave_base_value& a)
{
  CAST_CONV_ARG (const octave_perm_matrix&);

  return new octave_float_perm_matrix (v.perm_matrix_value ());
}

octave_base_value::type_conv_info
octave_perm_matrix::numeric_demotion_function (void) const
{
  return octave_base_value::type_conv_info (default_numeric_demotion_function,
                                            octave_float_perm_matrix::static_type_id ());
}

octave_base_value *
octave_perm_matrix::try_narrowing_conversion (void)
{
  octave_base_value *retval = 0;

  if (matrix.nelem () == 1)
    retval = new octave_scalar (matrix (0, 0));

  return retval;
}

