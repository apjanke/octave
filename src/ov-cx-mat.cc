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
Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <vector>

#include "data-conv.h"
#include "lo-ieee.h"
#include "mx-base.h"
#include "mach-info.h"

#include "gripes.h"
#include "oct-obj.h"
#include "oct-stream.h"
#include "ops.h"
#include "ov-base.h"
#include "ov-base-mat.h"
#include "ov-base-mat.cc"
#include "ov-complex.h"
#include "ov-cx-mat.h"
#include "ov-re-mat.h"
#include "ov-scalar.h"
#include "pr-output.h"

#include "byte-swap.h"
#include "ls-oct-ascii.h"
#include "ls-hdf5.h"
#include "ls-utils.h"

template class octave_base_matrix<ComplexNDArray>;

DEFINE_OCTAVE_ALLOCATOR (octave_complex_matrix);

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA (octave_complex_matrix,
				     "complex matrix", "double");

octave_base_value *
octave_complex_matrix::try_narrowing_conversion (void)
{
  octave_base_value *retval = 0;

  if (matrix.ndims () == 2)
    {
      ComplexMatrix cm = matrix.matrix_value ();

      octave_idx_type nr = cm.rows ();
      octave_idx_type nc = cm.cols ();

      if (nr == 1 && nc == 1)
	{
	  Complex c = matrix (0, 0);

	  double im = std::imag (c);

	  if (im == 0.0 && ! lo_ieee_signbit (im))
	    retval = new octave_scalar (std::real (c));
	  else
	    retval = new octave_complex (c);
	}
      else if (nr == 0 || nc == 0)
	retval = new octave_matrix (Matrix (nr, nc));
      else if (cm.all_elements_are_real ())
	retval = new octave_matrix (::real (cm));
    }
  else if (matrix.all_elements_are_real ())
    retval = new octave_matrix (::real (matrix));

  return retval;
}

void
octave_complex_matrix::assign (const octave_value_list& idx,
			       const ComplexNDArray& rhs)
{
  octave_base_matrix<ComplexNDArray>::assign (idx, rhs);
}

void
octave_complex_matrix::assign (const octave_value_list& idx,
			       const NDArray& rhs)
{
  octave_idx_type len = idx.length ();

  for (octave_idx_type i = 0; i < len; i++)
    matrix.set_index (idx(i).index_vector ());

  ::assign (matrix, rhs);
}

bool
octave_complex_matrix::valid_as_scalar_index (void) const
{
  // FIXME
  return false;
}

double
octave_complex_matrix::double_value (bool force_conversion) const
{
  double retval = lo_ieee_nan_value ();

  if (! force_conversion)
    gripe_implicit_conversion ("Octave:imag-to-real",
			       "complex matrix", "real scalar");

  if (rows () > 0 && columns () > 0)
    {
      gripe_implicit_conversion ("Octave:array-as-scalar",
				 "complex matrix", "real scalar");

      retval = std::real (matrix (0, 0));
    }
  else
    gripe_invalid_conversion ("complex matrix", "real scalar");

  return retval;
}

Matrix
octave_complex_matrix::matrix_value (bool force_conversion) const
{
  Matrix retval;

  if (! force_conversion)
    gripe_implicit_conversion ("Octave:imag-to-real",
			       "complex matrix", "real matrix");

  retval = ::real (matrix.matrix_value ());

  return retval;
}

Complex
octave_complex_matrix::complex_value (bool) const
{
  double tmp = lo_ieee_nan_value ();

  Complex retval (tmp, tmp);

  if (rows () > 0 && columns () > 0)
    {
      gripe_implicit_conversion ("Octave:array-as-scalar",
				 "complex matrix", "complex scalar");

      retval = matrix (0, 0);
    }
  else
    gripe_invalid_conversion ("complex matrix", "complex scalar");

  return retval;
}

ComplexMatrix
octave_complex_matrix::complex_matrix_value (bool) const
{
  return matrix.matrix_value ();
}

SparseMatrix
octave_complex_matrix::sparse_matrix_value (bool force_conversion) const
{
  SparseMatrix retval;

  if (! force_conversion)
    gripe_implicit_conversion ("Octave:imag-to-real",
			       "complex matrix", "real matrix");

  retval = SparseMatrix (::real (matrix.matrix_value ()));

  return retval;
}

SparseComplexMatrix
octave_complex_matrix::sparse_complex_matrix_value (bool) const
{
  return SparseComplexMatrix (matrix.matrix_value ());
}

static ComplexMatrix
strip_infnan (const ComplexMatrix& m)
{
  octave_idx_type nr = m.rows ();
  octave_idx_type nc = m.columns ();

  ComplexMatrix retval (nr, nc);

  octave_idx_type k = 0;
  for (octave_idx_type i = 0; i < nr; i++)
    {
      for (octave_idx_type j = 0; j < nc; j++)
	{
	  Complex c = m (i, j);
	  if (xisnan (c))
	    goto next_row;
	  else
	    {
	      double re = std::real (c);
	      double im = std::imag (c);

	      re = xisinf (re) ? (re > 0 ? OCT_RBV : -OCT_RBV) : re;
	      im = xisinf (im) ? (im > 0 ? OCT_RBV : -OCT_RBV) : im;

	      retval (k, j) = Complex (re, im);
	    }
	}
      k++;

    next_row:
      continue;
    }

  if (k > 0)
    retval.resize (k, nc);

  return retval;
}

bool 
octave_complex_matrix::save_ascii (std::ostream& os, bool& infnan_warned, 
			       bool strip_nan_and_inf)
{
  dim_vector d = dims ();
  if (d.length () > 2)
    {
      ComplexNDArray tmp = complex_array_value ();

      if (strip_nan_and_inf)
	{
	  warning ("save: Can not strip Inf or NaN values");
	  warning ("save: Inf or NaN values may not be reloadable");
	  infnan_warned = true;
	}
      else if (! infnan_warned && tmp.any_element_is_inf_or_nan ())
	{
	  warning ("save: Inf or NaN values may not be reloadable");
	  infnan_warned = true;
	}

      os << "# ndims: " << d.length () << "\n";

      for (int i = 0; i < d.length (); i++)
	os << " " << d (i);

      os << "\n" << tmp;
    }
  else
    {
      // Keep this case, rather than use generic code above for backward 
      // compatiability. Makes load_ascii much more complex!!
      os << "# rows: " << rows () << "\n"
	 << "# columns: " << columns () << "\n";

      ComplexMatrix tmp = complex_matrix_value ();

      if (strip_nan_and_inf)
	tmp = strip_infnan (tmp);
      else if (! infnan_warned && tmp.any_element_is_inf_or_nan ())
	{
	  warning ("save: Inf or NaN values may not be reloadable");
	  infnan_warned = true;
	}

      os << tmp;
    }

  return true;
}

bool 
octave_complex_matrix::load_ascii (std::istream& is)
{
  bool success = true;

  string_vector keywords(2);

  keywords[0] = "ndims";
  keywords[1] = "rows";

  std::string kw;
  octave_idx_type val = 0;

  if (extract_keyword (is, keywords, kw, val, true))
    {
      if (kw == "ndims")
	{
	  int mdims = static_cast<int> (val);

	  if (mdims >= 0)
	    {
	      dim_vector dv;
	      dv.resize (mdims);

	      for (int i = 0; i < mdims; i++)
		is >> dv(i);

	      ComplexNDArray tmp(dv);
	      is >> tmp;

	      if (!is) 
		{
		  error ("load: failed to load matrix constant");
		  success = false;
		}
	      matrix = tmp;
	    }
	  else
	    {
	      error ("load: failed to extract number of rows and columns");
	      success = false;
	    }
	}
      else if (kw == "rows")
	{
	  octave_idx_type nr = val;
	  octave_idx_type nc = 0;

	  if (nr >= 0 && extract_keyword (is, "columns", nc) && nc >= 0)
	    {
	      if (nr > 0 && nc > 0)
		{
		  ComplexMatrix tmp (nr, nc);
		  is >> tmp;
		  if (!is)
		    {
		      error ("load: failed to load matrix constant");
		      success = false;
		    }
		  matrix = tmp;
		}
	      else if (nr == 0 || nc == 0)
		matrix = ComplexMatrix (nr, nc);
	      else
		panic_impossible ();
	    }
	  else
	    {
	      error ("load: failed to extract number of rows and columns");
	      success = false;
	    }
	}
      else
	panic_impossible ();
    }
  else
    {
      error ("load: failed to extract number of rows and columns");
      success = false;
    }

  return success;
}

bool 
octave_complex_matrix::save_binary (std::ostream& os, bool& save_as_floats)
{
  dim_vector d = dims ();
  if (d.length() < 1)
    return false;

  // Use negative value for ndims to differentiate with old format!!
  int32_t tmp = - d.length();
  os.write (reinterpret_cast<char *> (&tmp), 4);
  for (int i = 0; i < d.length (); i++)
    {
      tmp = d(i);
      os.write (reinterpret_cast<char *> (&tmp), 4);
    }

  ComplexNDArray m = complex_array_value ();
  save_type st = LS_DOUBLE;
  if (save_as_floats)
    {
      if (m.too_large_for_float ())
	{
	  warning ("save: some values too large to save as floats --");
	  warning ("save: saving as doubles instead");
	}
      else
	st = LS_FLOAT;
    }
  else if (d.numel () > 4096) // FIXME -- make this configurable.
    {
      double max_val, min_val;
      if (m.all_integers (max_val, min_val))
	st = get_save_type (max_val, min_val);
    }


  const Complex *mtmp = m.data ();
  write_doubles (os, reinterpret_cast<const double *> (mtmp), st, 2 * d.numel ());

  return true;
}

bool 
octave_complex_matrix::load_binary (std::istream& is, bool swap,
				 oct_mach_info::float_format fmt)
{
  char tmp;
  int32_t mdims;
  if (! is.read (reinterpret_cast<char *> (&mdims), 4))
    return false;
  if (swap)
    swap_bytes<4> (&mdims);
  if (mdims < 0)
    {
      mdims = - mdims;
      int32_t di;
      dim_vector dv;
      dv.resize (mdims);

      for (int i = 0; i < mdims; i++)
	{
	  if (! is.read (reinterpret_cast<char *> (&di), 4))
	    return false;
	  if (swap)
	    swap_bytes<4> (&di);
	  dv(i) = di;
	}

      // Convert an array with a single dimension to be a row vector.
      // Octave should never write files like this, other software
      // might.

      if (mdims == 1)
	{
	  mdims = 2;
	  dv.resize (mdims);
	  dv(1) = dv(0);
	  dv(0) = 1;
	}

      if (! is.read (reinterpret_cast<char *> (&tmp), 1))
	return false;

      ComplexNDArray m(dv);
      Complex *im = m.fortran_vec ();
      read_doubles (is, reinterpret_cast<double *> (im),
		    static_cast<save_type> (tmp), 2 * dv.numel (), swap, fmt);
      if (error_state || ! is)
	return false;
      matrix = m;
    }
  else
    {
      int32_t nr, nc;
      nr = mdims;
      if (! is.read (reinterpret_cast<char *> (&nc), 4))
	return false;
      if (swap)
	swap_bytes<4> (&nc);
      if (! is.read (reinterpret_cast<char *> (&tmp), 1))
	return false;
      ComplexMatrix m (nr, nc);
      Complex *im = m.fortran_vec ();
      octave_idx_type len = nr * nc;
      read_doubles (is, reinterpret_cast<double *> (im),
		    static_cast<save_type> (tmp), 2*len, swap, fmt);
      if (error_state || ! is)
	return false;
      matrix = m;
    }
  return true;
}

#if defined (HAVE_HDF5)

bool
octave_complex_matrix::save_hdf5 (hid_t loc_id, const char *name,
				  bool save_as_floats)
{
  dim_vector dv = dims ();
  int empty = save_hdf5_empty (loc_id, name, dv);
  if (empty)
    return (empty > 0);

  int rank = dv.length ();
  hid_t space_hid = -1, data_hid = -1, type_hid = -1;
  bool retval = true;
  ComplexNDArray m = complex_array_value ();

  OCTAVE_LOCAL_BUFFER (hsize_t, hdims, rank);

  // Octave uses column-major, while HDF5 uses row-major ordering
  for (int i = 0; i < rank; i++)
    hdims[i] = dv (rank-i-1);
 
  space_hid = H5Screate_simple (rank, hdims, 0);
  if (space_hid < 0) return false;

  hid_t save_type_hid = H5T_NATIVE_DOUBLE;

  if (save_as_floats)
    {
      if (m.too_large_for_float ())
	{
	  warning ("save: some values too large to save as floats --");
	  warning ("save: saving as doubles instead");
	}
      else
	save_type_hid = H5T_NATIVE_FLOAT;
    }
#if HAVE_HDF5_INT2FLOAT_CONVERSIONS
  // hdf5 currently doesn't support float/integer conversions
  else
    {
      double max_val, min_val;
      
      if (m.all_integers (max_val, min_val))
	save_type_hid
	  = save_type_to_hdf5 (get_save_type (max_val, min_val));
    }
#endif /* HAVE_HDF5_INT2FLOAT_CONVERSIONS */

  type_hid = hdf5_make_complex_type (save_type_hid);
  if (type_hid < 0)
    {
      H5Sclose (space_hid);
      return false;
    }

  data_hid = H5Dcreate (loc_id, name, type_hid, space_hid, H5P_DEFAULT);
  if (data_hid < 0)
    {
      H5Sclose (space_hid);
      H5Tclose (type_hid);
      return false;
    }

  hid_t complex_type_hid = hdf5_make_complex_type (H5T_NATIVE_DOUBLE);
  if (complex_type_hid < 0) retval = false;

  if (retval)
    {
      Complex *mtmp = m.fortran_vec ();
      if (H5Dwrite (data_hid, complex_type_hid, H5S_ALL, H5S_ALL, H5P_DEFAULT,
		    mtmp) < 0)
	{
	  H5Tclose (complex_type_hid);
	  retval = false;
	}
    }

  H5Tclose (complex_type_hid);
  H5Dclose (data_hid);
  H5Tclose (type_hid);
  H5Sclose (space_hid);

  return retval;
}

bool 
octave_complex_matrix::load_hdf5 (hid_t loc_id, const char *name,
				  bool /* have_h5giterate_bug */)
{
  bool retval = false;

  dim_vector dv;
  int empty = load_hdf5_empty (loc_id, name, dv);
  if (empty > 0)
    matrix.resize(dv);
  if (empty)
      return (empty > 0);

  hid_t data_hid = H5Dopen (loc_id, name);
  hid_t type_hid = H5Dget_type (data_hid);

  hid_t complex_type = hdf5_make_complex_type (H5T_NATIVE_DOUBLE);

  if (! hdf5_types_compatible (type_hid, complex_type))
    {
      H5Tclose (complex_type);
      H5Dclose (data_hid);
      return false;
    }

  hid_t space_id = H5Dget_space (data_hid);

  hsize_t rank = H5Sget_simple_extent_ndims (space_id);
  
  if (rank < 1)
    {
      H5Tclose (complex_type);
      H5Sclose (space_id);
      H5Dclose (data_hid);
      return false;
    }

  OCTAVE_LOCAL_BUFFER (hsize_t, hdims, rank);
  OCTAVE_LOCAL_BUFFER (hsize_t, maxdims, rank);

  H5Sget_simple_extent_dims (space_id, hdims, maxdims);

  // Octave uses column-major, while HDF5 uses row-major ordering
  if (rank == 1)
    {
      dv.resize (2);
      dv(0) = 1;
      dv(1) = hdims[0];
    }
  else
    {
      dv.resize (rank);
      for (hsize_t i = 0, j = rank - 1; i < rank; i++, j--)
	dv(j) = hdims[i];
    }

  ComplexNDArray m (dv);
  Complex *reim = m.fortran_vec ();
  if (H5Dread (data_hid, complex_type, H5S_ALL, H5S_ALL, H5P_DEFAULT,
	       reim) >= 0) 
    {
      retval = true;
      matrix = m;
    }

  H5Tclose (complex_type);
  H5Sclose (space_id);
  H5Dclose (data_hid);

  return retval;
}

#endif

void
octave_complex_matrix::print_raw (std::ostream& os,
				  bool pr_as_read_syntax) const
{
  octave_print_internal (os, matrix, pr_as_read_syntax,
			 current_print_indent_level ());
}

mxArray *
octave_complex_matrix::as_mxArray (void) const
{
  mxArray *retval = new mxArray (mxDOUBLE_CLASS, dims (), mxCOMPLEX);

  double *pr = static_cast<double *> (retval->get_data ());
  double *pi = static_cast<double *> (retval->get_imag_data ());

  int nel = numel ();

  const Complex *p = matrix.data ();

  for (int i = 0; i < nel; i++)
    {
      pr[i] = real (p[i]);
      pi[i] = imag (p[i]);
    }

  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
