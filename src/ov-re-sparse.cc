/*

Copyright (C) 2004, 2005, 2006, 2007, 2008 David Bateman
Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004 Andy Adler

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

#include <climits>

#include <iostream>
#include <vector>

#include "lo-specfun.h"
#include "lo-mappers.h"
#include "oct-locbuf.h"

#include "ov-base.h"
#include "ov-scalar.h"
#include "gripes.h"

#include "ls-hdf5.h"

#include "ov-re-sparse.h"

#include "ov-base-sparse.h"
#include "ov-base-sparse.cc"

#include "ov-bool-sparse.h"

template class OCTINTERP_API octave_base_sparse<SparseMatrix>;

DEFINE_OCTAVE_ALLOCATOR (octave_sparse_matrix);

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA (octave_sparse_matrix, "sparse matrix", "double");

idx_vector
octave_sparse_matrix::index_vector (void) const
{
  if (matrix.numel () == matrix.nnz ())
    return idx_vector (array_value ());
  else
    {
      std::string nm = type_name ();
      error ("%s type invalid as index value", nm.c_str ());
      return idx_vector ();
    }
}

octave_base_value *
octave_sparse_matrix::try_narrowing_conversion (void)
{
  octave_base_value *retval = 0;

  if (Vsparse_auto_mutate)
    {
      // Don't use numel, since it can overflow for very large matrices
      // Note that for the second test, this means it becomes approximative
      // since it involves a cast to double to avoid issues of overflow
      if (matrix.rows () == 1 && matrix.cols () == 1)
	{
	  // Const copy of the matrix, so the right version of () operator used
	  const SparseMatrix tmp (matrix);

	  retval = new octave_scalar (tmp (0));
	}
      else if (matrix.cols () > 0 && matrix.rows () > 0
	       && (double (matrix.byte_size ()) > double (matrix.rows ())
		   * double (matrix.cols ()) * sizeof (double)))
	retval = new octave_matrix (matrix.matrix_value ());
    }

  return retval;
}

double
octave_sparse_matrix::double_value (bool) const
{
  double retval = lo_ieee_nan_value ();

  if (numel () > 0)
    {
      if (numel () > 1)
	gripe_implicit_conversion ("Octave:array-as-scalar",
				   "real sparse matrix", "real scalar");

      retval = matrix (0, 0);
    }
  else
    gripe_invalid_conversion ("real sparse matrix", "real scalar");

  return retval;
}

Complex
octave_sparse_matrix::complex_value (bool) const
{
  double tmp = lo_ieee_nan_value ();

  Complex retval (tmp, tmp);

  // FIXME -- maybe this should be a function, valid_as_scalar()
  if (rows () > 0 && columns () > 0)
    {
      if (numel () > 1)
	gripe_implicit_conversion ("Octave:array-as-scalar",
				   "real sparse matrix", "complex scalar");

      retval = matrix (0, 0);
    }
  else
    gripe_invalid_conversion ("real sparse matrix", "complex scalar");

  return retval;
}

Matrix
octave_sparse_matrix::matrix_value (bool) const
{
  return matrix.matrix_value ();
}

boolNDArray
octave_sparse_matrix::bool_array_value (bool warn) const
{
  NDArray m = matrix.matrix_value ();

  if (m.any_element_is_nan ())
    error ("invalid conversion from NaN to logical");
  else if (warn && m.any_element_not_one_or_zero ())
    gripe_logical_conversion ();

  return boolNDArray (m);
}

charNDArray
octave_sparse_matrix::char_array_value (bool) const
{
  charNDArray retval (dims (), 0);
  octave_idx_type nc = matrix.cols ();
  octave_idx_type nr = matrix.rows ();

  for (octave_idx_type j = 0; j < nc; j++)
    for (octave_idx_type i = matrix.cidx(j); i < matrix.cidx(j+1); i++)
      retval(matrix.ridx(i) + nr * j) = static_cast<char>(matrix.data (i));

  return retval;
}
  
ComplexMatrix
octave_sparse_matrix::complex_matrix_value (bool) const
{
  return ComplexMatrix (matrix.matrix_value ());
}

ComplexNDArray
octave_sparse_matrix::complex_array_value (bool) const
{
  return ComplexNDArray (ComplexMatrix (matrix.matrix_value ()));
}

NDArray 
octave_sparse_matrix::array_value (bool) const
{
  return NDArray (matrix.matrix_value ());
}

octave_value
octave_sparse_matrix::convert_to_str_internal (bool, bool, char type) const
{
  octave_value retval;
  dim_vector dv = dims ();
  octave_idx_type nel = dv.numel ();

  if (nel == 0)
    {
      char s = '\0';
      retval = octave_value (&s, type);
    }
  else
    {
      octave_idx_type nr = matrix.rows ();
      octave_idx_type nc = matrix.cols ();
      charNDArray chm (dv, static_cast<char> (0));
	  
      bool warned = false;

      for (octave_idx_type j = 0; j < nc; j++)
	for (octave_idx_type i = matrix.cidx(j); 
	     i < matrix.cidx(j+1); i++)
	  {
	    OCTAVE_QUIT;

	    double d = matrix.data (i);

	      if (xisnan (d))
		{
		  ::error ("invalid conversion from NaN to character");
		  return retval;
		}
	      else
		{
		  int ival = NINT (d);

		  if (ival < 0 || ival > UCHAR_MAX)
		    {
		      // FIXME -- is there something
		      // better we could do?

		      ival = 0;

		      if (! warned)
			{
			  ::warning ("range error for conversion to character value");
			  warned = true;
			}
		    }

		  chm (matrix.ridx(i) + j * nr) = 
		    static_cast<char> (ival);
		}
	  }
      retval = octave_value (chm, true, type);
    }

  return retval;
}

bool 
octave_sparse_matrix::save_binary (std::ostream& os, bool&save_as_floats)
{
  dim_vector d = this->dims ();
  if (d.length() < 1)
    return false;

  // Ensure that additional memory is deallocated
  matrix.maybe_compress ();

  int nr = d(0);
  int nc = d(1);
  int nz = nzmax ();

  int32_t itmp;
  // Use negative value for ndims to be consistent with other formats
  itmp= -2;        
  os.write (reinterpret_cast<char *> (&itmp), 4);
  
  itmp= nr;    
  os.write (reinterpret_cast<char *> (&itmp), 4);
  
  itmp= nc;
  os.write (reinterpret_cast<char *> (&itmp), 4);
  
  itmp= nz;
  os.write (reinterpret_cast<char *> (&itmp), 4);

  save_type st = LS_DOUBLE;
  if (save_as_floats)
    {
      if (matrix.too_large_for_float ())
	{
	  warning ("save: some values too large to save as floats --");
	  warning ("save: saving as doubles instead");
	}
      else
	st = LS_FLOAT;
    }
  else if (matrix.nzmax () > 8192) // FIXME -- make this configurable.
    {
      double max_val, min_val;
      if (matrix.all_integers (max_val, min_val))
	st = get_save_type (max_val, min_val);
    }

  // add one to the printed indices to go from
  // zero-based to one-based arrays
   for (int i = 0; i < nc+1; i++)  
     {
       OCTAVE_QUIT;
       itmp = matrix.cidx(i);
       os.write (reinterpret_cast<char *> (&itmp), 4);
     }

   for (int i = 0; i < nz; i++) 
     {
       OCTAVE_QUIT;
       itmp = matrix.ridx(i); 
       os.write (reinterpret_cast<char *> (&itmp), 4);
     }

   write_doubles (os, matrix.data(), st, nz);

  return true;
}

bool
octave_sparse_matrix::load_binary (std::istream& is, bool swap,
				   oct_mach_info::float_format fmt)
{
  int32_t nz, nc, nr, tmp;
  char ctmp;

  if (! is.read (reinterpret_cast<char *> (&tmp), 4))
    return false;

  if (swap)
    swap_bytes<4> (&tmp);

  if (tmp != -2) {
    error ("load: only 2D sparse matrices are supported");
    return false;
  }

  if (! is.read (reinterpret_cast<char *> (&nr), 4))
    return false;
  if (! is.read (reinterpret_cast<char *> (&nc), 4))
    return false;
  if (! is.read (reinterpret_cast<char *> (&nz), 4))
    return false;

  if (swap)
    {
      swap_bytes<4> (&nr);
      swap_bytes<4> (&nc);
      swap_bytes<4> (&nz);
    }

  SparseMatrix m (static_cast<octave_idx_type> (nr),
		  static_cast<octave_idx_type> (nc),
		  static_cast<octave_idx_type> (nz));

  for (int i = 0; i < nc+1; i++) 
    {
      OCTAVE_QUIT;
      if (! is.read (reinterpret_cast<char *> (&tmp), 4))
	return false;
      if (swap)
	swap_bytes<4> (&tmp);
      m.xcidx(i) = tmp;
    }

  for (int i = 0; i < nz; i++) 
    {
      OCTAVE_QUIT;
      if (! is.read (reinterpret_cast<char *> (&tmp), 4))
	return false;
      if (swap)
	swap_bytes<4> (&tmp);
      m.xridx(i) = tmp;
    }

  if (! is.read (reinterpret_cast<char *> (&ctmp), 1))
    return false;
  
  read_doubles (is, m.xdata (), static_cast<save_type> (ctmp), nz, swap, fmt);

  if (error_state || ! is)
    return false;

  if (! m.indices_ok ())
    return false;

  matrix = m;

  return true;
}

#if defined (HAVE_HDF5)

bool
octave_sparse_matrix::save_hdf5 (hid_t loc_id, const char *name, 
				 bool save_as_floats)
{
  dim_vector dv = dims ();
  int empty = save_hdf5_empty (loc_id, name, dv);
  if (empty)
    return (empty > 0);

  // Ensure that additional memory is deallocated
  matrix.maybe_compress ();

  hid_t group_hid = H5Gcreate (loc_id, name, 0);
  if (group_hid < 0)
    return false;

  hid_t space_hid = -1, data_hid = -1;
  bool retval = true;
  SparseMatrix m = sparse_matrix_value ();
  octave_idx_type tmp;
  hsize_t hdims[2];

  space_hid = H5Screate_simple (0, hdims, 0);
  if (space_hid < 0) 
    {
      H5Gclose (group_hid);
      return false;
    }

  data_hid = H5Dcreate (group_hid, "nr", H5T_NATIVE_IDX, space_hid, 
			H5P_DEFAULT);
  if (data_hid < 0) 
    {
      H5Sclose (space_hid);
      H5Gclose (group_hid);
      return false;
    }
  
  tmp = m.rows ();
  retval = H5Dwrite (data_hid, H5T_NATIVE_IDX, H5S_ALL, H5S_ALL, H5P_DEFAULT,
		     &tmp) >= 0;
  H5Dclose (data_hid);
  if (!retval)
    {
      H5Sclose (space_hid);
      H5Gclose (group_hid);
      return false;
    }    

  data_hid = H5Dcreate (group_hid, "nc", H5T_NATIVE_IDX, space_hid, 
			H5P_DEFAULT);
  if (data_hid < 0) 
    {
      H5Sclose (space_hid);
      H5Gclose (group_hid);
      return false;
    }
  
  tmp = m.cols ();
  retval = H5Dwrite (data_hid, H5T_NATIVE_IDX, H5S_ALL, H5S_ALL, H5P_DEFAULT,
		     &tmp) >= 0;
  H5Dclose (data_hid);
  if (!retval)
    {
      H5Sclose (space_hid);
      H5Gclose (group_hid);
      return false;
    }    

  data_hid = H5Dcreate (group_hid, "nz", H5T_NATIVE_IDX, space_hid, 
			H5P_DEFAULT);
  if (data_hid < 0) 
    {
      H5Sclose (space_hid);
      H5Gclose (group_hid);
      return false;
    }
  
  tmp = m.nzmax ();
  retval = H5Dwrite (data_hid, H5T_NATIVE_IDX, H5S_ALL, H5S_ALL, H5P_DEFAULT,
		     &tmp) >= 0;
  H5Dclose (data_hid);
  if (!retval)
    {
      H5Sclose (space_hid);
      H5Gclose (group_hid);
      return false;
    }

  H5Sclose (space_hid);

  hdims[0] = m.cols() + 1;
  hdims[1] = 1;

  space_hid = H5Screate_simple (2, hdims, 0);

  if (space_hid < 0) 
    {
      H5Gclose (group_hid);
      return false;
    }

  data_hid = H5Dcreate (group_hid, "cidx", H5T_NATIVE_IDX, space_hid, 
			H5P_DEFAULT);
  if (data_hid < 0) 
    {
      H5Sclose (space_hid);
      H5Gclose (group_hid);
      return false;
    }
  
  octave_idx_type * itmp = m.xcidx ();
  retval = H5Dwrite (data_hid, H5T_NATIVE_IDX, H5S_ALL, H5S_ALL, H5P_DEFAULT,
		     itmp) >= 0;
  H5Dclose (data_hid);
  if (!retval)
    {
      H5Sclose (space_hid);
      H5Gclose (group_hid);
      return false;
    }    

  H5Sclose (space_hid);

  hdims[0] = m.nzmax ();
  hdims[1] = 1;

  space_hid = H5Screate_simple (2, hdims, 0);

  if (space_hid < 0) 
    {
      H5Gclose (group_hid);
      return false;
    }

  data_hid = H5Dcreate (group_hid, "ridx", H5T_NATIVE_IDX, space_hid, 
			H5P_DEFAULT);
  if (data_hid < 0) 
    {
      H5Sclose (space_hid);
      H5Gclose (group_hid);
      return false;
    }
  
  itmp = m.xridx ();
  retval = H5Dwrite (data_hid, H5T_NATIVE_IDX, H5S_ALL, H5S_ALL, H5P_DEFAULT,
		     itmp) >= 0;
  H5Dclose (data_hid);
  if (!retval)
    {
      H5Sclose (space_hid);
      H5Gclose (group_hid);
      return false;
    }

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

  data_hid = H5Dcreate (group_hid, "data", save_type_hid, space_hid, 
			H5P_DEFAULT);
  if (data_hid < 0) 
    {
      H5Sclose (space_hid);
      H5Gclose (group_hid);
      return false;
    }
  
  double * dtmp = m.xdata ();
  retval = H5Dwrite (data_hid, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
		     H5P_DEFAULT, dtmp) >= 0;
  H5Dclose (data_hid);
  H5Sclose (space_hid);
  H5Gclose (group_hid);

  return retval;
}

bool
octave_sparse_matrix::load_hdf5 (hid_t loc_id, const char *name,
				 bool /* have_h5giterate_bug */)
{
  octave_idx_type nr, nc, nz;
  hid_t group_hid, data_hid, space_hid;
  hsize_t rank;
  
  dim_vector dv;
  int empty = load_hdf5_empty (loc_id, name, dv);
  if (empty > 0)
    matrix.resize(dv);
  if (empty)
    return (empty > 0);
  
  group_hid = H5Gopen (loc_id, name);
  if (group_hid < 0) return false;

  data_hid = H5Dopen (group_hid, "nr");
  space_hid = H5Dget_space (data_hid);
  rank = H5Sget_simple_extent_ndims (space_hid);

  if (rank != 0)
    { 
      H5Dclose (data_hid);
      H5Gclose (group_hid);
      return false;
    }

  if (H5Dread (data_hid, H5T_NATIVE_IDX, H5S_ALL, H5S_ALL, 
	       H5P_DEFAULT, &nr) < 0)
    { 
      H5Dclose (data_hid);
      H5Gclose (group_hid);
      return false;
    }

  H5Dclose (data_hid);

  data_hid = H5Dopen (group_hid, "nc");
  space_hid = H5Dget_space (data_hid);
  rank = H5Sget_simple_extent_ndims (space_hid);

  if (rank != 0)
    { 
      H5Dclose (data_hid);
      H5Gclose (group_hid);
      return false;
    }

  if (H5Dread (data_hid, H5T_NATIVE_IDX, H5S_ALL, H5S_ALL, 
	       H5P_DEFAULT, &nc) < 0)
    { 
      H5Dclose (data_hid);
      H5Gclose (group_hid);
      return false;
    }

  H5Dclose (data_hid);
  
  data_hid = H5Dopen (group_hid, "nz");
  space_hid = H5Dget_space (data_hid);
  rank = H5Sget_simple_extent_ndims (space_hid);

  if (rank != 0)
    { 
      H5Dclose (data_hid);
      H5Gclose (group_hid);
      return false;
    }

  if (H5Dread (data_hid, H5T_NATIVE_IDX, H5S_ALL, H5S_ALL, 
	       H5P_DEFAULT, &nz) < 0)
    { 
      H5Dclose (data_hid);
      H5Gclose (group_hid);
      return false;
    }

  H5Dclose (data_hid);

  SparseMatrix m (static_cast<octave_idx_type> (nr),
		  static_cast<octave_idx_type> (nc),
		  static_cast<octave_idx_type> (nz));

  data_hid = H5Dopen (group_hid, "cidx");
  space_hid = H5Dget_space (data_hid);
  rank = H5Sget_simple_extent_ndims (space_hid);

  if (rank != 2)
    {
      H5Sclose (space_hid);
      H5Dclose (data_hid);
      H5Gclose (group_hid);
      return false;
    }

  OCTAVE_LOCAL_BUFFER (hsize_t, hdims, rank);
  OCTAVE_LOCAL_BUFFER (hsize_t, maxdims, rank);

  H5Sget_simple_extent_dims (space_hid, hdims, maxdims);

  if (static_cast<int> (hdims[0]) != nc + 1
      || static_cast<int> (hdims[1]) != 1)
    {
      H5Sclose (space_hid);
      H5Dclose (data_hid);
      H5Gclose (group_hid);
      return false;
    }

  octave_idx_type *itmp = m.xcidx ();
  if (H5Dread (data_hid, H5T_NATIVE_IDX, H5S_ALL, H5S_ALL, 
	       H5P_DEFAULT, itmp) < 0) 
    {
      H5Sclose (space_hid);
      H5Dclose (data_hid);
      H5Gclose (group_hid);
      return false;
    }

  H5Sclose (space_hid);
  H5Dclose (data_hid);

  data_hid = H5Dopen (group_hid, "ridx");
  space_hid = H5Dget_space (data_hid);
  rank = H5Sget_simple_extent_ndims (space_hid);

  if (rank != 2)
    {
      H5Sclose (space_hid);
      H5Dclose (data_hid);
      H5Gclose (group_hid);
      return false;
    }

  H5Sget_simple_extent_dims (space_hid, hdims, maxdims);

  if (static_cast<int> (hdims[0]) != nz || static_cast<int> (hdims[1]) != 1)
    {
      H5Sclose (space_hid);
      H5Dclose (data_hid);
      H5Gclose (group_hid);
      return false;
    }

  itmp = m.xridx ();
  if (H5Dread (data_hid, H5T_NATIVE_IDX, H5S_ALL, H5S_ALL, 
	       H5P_DEFAULT, itmp) < 0) 
    {
      H5Sclose (space_hid);
      H5Dclose (data_hid);
      H5Gclose (group_hid);
      return false;
    }

  H5Sclose (space_hid);
  H5Dclose (data_hid);

  data_hid = H5Dopen (group_hid, "data");
  space_hid = H5Dget_space (data_hid);
  rank = H5Sget_simple_extent_ndims (space_hid);

  if (rank != 2)
    {
      H5Sclose (space_hid);
      H5Dclose (data_hid);
      H5Gclose (group_hid);
      return false;
    }

  H5Sget_simple_extent_dims (space_hid, hdims, maxdims);

  if (static_cast<int> (hdims[0]) != nz || static_cast<int> (hdims[1]) != 1)
    {
      H5Sclose (space_hid);
      H5Dclose (data_hid);
      H5Gclose (group_hid);
      return false;
    }

  double *dtmp = m.xdata ();
  bool retval = false;
  if (H5Dread (data_hid, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, 
	       H5P_DEFAULT, dtmp) >= 0
      && m.indices_ok ())
    {
      retval = true;
      matrix = m;
    }

  H5Sclose (space_hid);
  H5Dclose (data_hid);
  H5Gclose (group_hid);

  return retval;
}

#endif

mxArray *
octave_sparse_matrix::as_mxArray (void) const
{
  mwSize nz = nzmax();
  mwSize nr = rows();
  mwSize nc = columns();
  mxArray *retval = new mxArray (mxDOUBLE_CLASS, nr, nc, nz, mxREAL);
  double *pr = static_cast<double *> (retval->get_data ());
  mwIndex *ir = retval->get_ir();
  mwIndex *jc = retval->get_jc();

  for (mwIndex i = 0; i < nz; i++)
    {
      pr[i] = matrix.data(i);
      ir[i] = matrix.ridx(i);
    }

  for (mwIndex i = 0; i < nc + 1; i++)
    jc[i] = matrix.cidx(i);

  return retval;
}

static bool
any_element_less_than (const SparseMatrix& a, double val)
{
  octave_idx_type len = a.nnz ();

  if (val > 0. && len != a.numel ())
    return true;

  for (octave_idx_type i = 0; i < len; i++)
    {
      OCTAVE_QUIT;

      if (a.data(i) < val)
	return true;
    }

  return false;
}

static bool
any_element_greater_than (const SparseMatrix& a, double val)
{
  octave_idx_type len = a.nnz ();

  if (val < 0. && len != a.numel ())
    return true;

  for (octave_idx_type i = 0; i < len; i++)
    {
      OCTAVE_QUIT;

      if (a.data(i) > val)
	return true;
    }

  return false;
}

#define SPARSE_MAPPER(MAP, AMAP, FCN) \
  octave_value \
  octave_sparse_matrix::MAP (void) const \
  { \
    static AMAP dmap = FCN; \
    return matrix.map (dmap); \
  }

#define CD_SPARSE_MAPPER(MAP, RFCN, CFCN, L1, L2) \
  octave_value \
  octave_sparse_matrix::MAP (void) const \
  { \
    static SparseMatrix::dmapper dmap = RFCN; \
    static SparseMatrix::cmapper cmap = CFCN; \
 \
    return (any_element_less_than (matrix, L1) \
            ? octave_value (matrix.map (cmap)) \
            : (any_element_greater_than (matrix, L2) \
               ? octave_value (matrix.map (cmap)) \
	       : octave_value (matrix.map (dmap)))); \
  }

static double
xconj (double x)
{
  return x;
}

SPARSE_MAPPER (erf, SparseMatrix::dmapper, ::erf)
SPARSE_MAPPER (erfc, SparseMatrix::dmapper, ::erfc)
SPARSE_MAPPER (gamma, SparseMatrix::dmapper, xgamma)
CD_SPARSE_MAPPER (lgamma, xlgamma, xlgamma, 0.0, octave_Inf)
SPARSE_MAPPER (abs, SparseMatrix::dmapper, ::fabs)
CD_SPARSE_MAPPER (acos, ::acos, ::acos, -1.0, 1.0)
CD_SPARSE_MAPPER (acosh, ::acosh, ::acosh, 1.0, octave_Inf)
SPARSE_MAPPER (angle, SparseMatrix::dmapper, ::arg)
SPARSE_MAPPER (arg, SparseMatrix::dmapper, ::arg)
CD_SPARSE_MAPPER (asin, ::asin, ::asin, -1.0, 1.0)
SPARSE_MAPPER (asinh, SparseMatrix::dmapper, ::asinh)
SPARSE_MAPPER (atan, SparseMatrix::dmapper, ::atan)
CD_SPARSE_MAPPER (atanh, ::atanh, ::atanh, -1.0, 1.0)
SPARSE_MAPPER (ceil, SparseMatrix::dmapper, ::ceil)
SPARSE_MAPPER (conj, SparseMatrix::dmapper, xconj)
SPARSE_MAPPER (cos, SparseMatrix::dmapper, ::cos)
SPARSE_MAPPER (cosh, SparseMatrix::dmapper, ::cosh)
SPARSE_MAPPER (exp, SparseMatrix::dmapper, ::exp)
SPARSE_MAPPER (expm1, SparseMatrix::dmapper, ::expm1)
SPARSE_MAPPER (fix, SparseMatrix::dmapper, ::fix)
SPARSE_MAPPER (floor, SparseMatrix::dmapper, ::floor)
SPARSE_MAPPER (imag, SparseMatrix::dmapper, ::imag)
CD_SPARSE_MAPPER (log, ::log, std::log, 0.0, octave_Inf)
CD_SPARSE_MAPPER (log2, xlog2, xlog2, 0.0, octave_Inf)
CD_SPARSE_MAPPER (log10, ::log10, std::log10, 0.0, octave_Inf)
CD_SPARSE_MAPPER (log1p, ::log1p, ::log1p, 0.0, octave_Inf)
SPARSE_MAPPER (real, SparseMatrix::dmapper, ::real)
SPARSE_MAPPER (round, SparseMatrix::dmapper, xround)
SPARSE_MAPPER (roundb, SparseMatrix::dmapper, xroundb)
SPARSE_MAPPER (signum, SparseMatrix::dmapper, ::signum)
SPARSE_MAPPER (sin, SparseMatrix::dmapper, ::sin)
SPARSE_MAPPER (sinh, SparseMatrix::dmapper, ::sinh)
CD_SPARSE_MAPPER (sqrt, ::sqrt, std::sqrt, 0.0, octave_Inf)
SPARSE_MAPPER (tan, SparseMatrix::dmapper, ::tan)
SPARSE_MAPPER (tanh, SparseMatrix::dmapper, ::tanh)
SPARSE_MAPPER (finite, SparseMatrix::bmapper, xfinite)
SPARSE_MAPPER (isinf, SparseMatrix::bmapper, xisinf)
SPARSE_MAPPER (isna, SparseMatrix::bmapper, octave_is_NA)
SPARSE_MAPPER (isnan, SparseMatrix::bmapper, xisnan)

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
