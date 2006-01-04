/*

Copyright (C) 2004 David Bateman
Copyright (C) 1998-2004 Andy Adler

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cfloat>

#include <iostream>
#include <vector>

#include "quit.h"
#include "lo-ieee.h"
#include "lo-mappers.h"
#include "f77-fcn.h"
#include "dRowVector.h"

#include "CSparse.h"
#include "boolSparse.h"
#include "dSparse.h"
#include "oct-spparms.h"
#include "SparseCmplxLU.h"
#include "oct-sparse.h"
#include "sparse-util.h"
#include "SparseCmplxCHOL.h"

#include "oct-sort.h"

// Fortran functions we call.
extern "C"
{
  F77_RET_T
  F77_FUNC (zgbtrf, ZGBTRF) (const octave_idx_type&, const octave_idx_type&, const octave_idx_type&, 
			     const octave_idx_type&, Complex*, const octave_idx_type&, octave_idx_type*, octave_idx_type&);

  F77_RET_T
  F77_FUNC (zgbtrs, ZGBTRS) (F77_CONST_CHAR_ARG_DECL, const octave_idx_type&,
			     const octave_idx_type&, const octave_idx_type&, const octave_idx_type&, 
			     const Complex*, const octave_idx_type&,
			     const octave_idx_type*, Complex*, const octave_idx_type&, octave_idx_type&
			     F77_CHAR_ARG_LEN_DECL);

  F77_RET_T
  F77_FUNC (zgbcon, ZGBCON) (F77_CONST_CHAR_ARG_DECL, const octave_idx_type&, 
			     const octave_idx_type&, const octave_idx_type&, Complex*, 
			     const octave_idx_type&, const octave_idx_type*, const double&, 
			     double&, Complex*, double*, octave_idx_type&
			     F77_CHAR_ARG_LEN_DECL);

  F77_RET_T
  F77_FUNC (zpbtrf, ZPBTRF) (F77_CONST_CHAR_ARG_DECL, const octave_idx_type&, 
			     const octave_idx_type&, Complex*, const octave_idx_type&, octave_idx_type&
			     F77_CHAR_ARG_LEN_DECL);

  F77_RET_T
  F77_FUNC (zpbtrs, ZPBTRS) (F77_CONST_CHAR_ARG_DECL, const octave_idx_type&, 
			     const octave_idx_type&, const octave_idx_type&, Complex*, const octave_idx_type&, 
			     Complex*, const octave_idx_type&, octave_idx_type&
			     F77_CHAR_ARG_LEN_DECL);

  F77_RET_T
  F77_FUNC (zpbcon, ZPBCON) (F77_CONST_CHAR_ARG_DECL, const octave_idx_type&, 
			     const octave_idx_type&, Complex*, const octave_idx_type&, 
			     const double&, double&, Complex*, octave_idx_type*, octave_idx_type&
			     F77_CHAR_ARG_LEN_DECL);

  F77_RET_T
  F77_FUNC (zgttrf, ZGTTRF) (const octave_idx_type&, Complex*, Complex*, Complex*,
			     Complex*, octave_idx_type*, octave_idx_type&);

  F77_RET_T
  F77_FUNC (zgttrs, ZGTTRS) (F77_CONST_CHAR_ARG_DECL, const octave_idx_type&,
			     const octave_idx_type&, const Complex*, const Complex*,
			     const Complex*, const Complex*, const octave_idx_type*,
			     Complex *, const octave_idx_type&, octave_idx_type&
			     F77_CHAR_ARG_LEN_DECL);

  F77_RET_T
  F77_FUNC (zptsv, ZPTSV) (const octave_idx_type&, const octave_idx_type&, double*, Complex*,
			   Complex*, const octave_idx_type&, octave_idx_type&);

  F77_RET_T
  F77_FUNC (zgtsv, ZGTSV) (const octave_idx_type&, const octave_idx_type&, Complex*, Complex*,
			   Complex*, Complex*, const octave_idx_type&, octave_idx_type&);
}

SparseComplexMatrix::SparseComplexMatrix (const SparseMatrix& a)
  : MSparse<Complex> (a.rows (), a.cols (), a.nnz ())
{
  octave_idx_type nc = cols ();
  octave_idx_type nz = nnz ();

  for (octave_idx_type i = 0; i < nc + 1; i++)
    cidx (i) = a.cidx (i);

  for (octave_idx_type i = 0; i < nz; i++)
    {
      data (i) = a.data (i);
      ridx (i) = a.ridx (i);
    }
}

SparseComplexMatrix::SparseComplexMatrix (const SparseBoolMatrix& a)
  : MSparse<Complex> (a.rows (), a.cols (), a.nnz ())
{
  octave_idx_type nc = cols ();
  octave_idx_type nz = nnz ();

  for (octave_idx_type i = 0; i < nc + 1; i++)
    cidx (i) = a.cidx (i);

  for (octave_idx_type i = 0; i < nz; i++)
    {
      data (i) = a.data (i);
      ridx (i) = a.ridx (i);
    }
}

bool
SparseComplexMatrix::operator == (const SparseComplexMatrix& a) const
{
  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  octave_idx_type nz = nnz ();
  octave_idx_type nr_a = a.rows ();
  octave_idx_type nc_a = a.cols ();
  octave_idx_type nz_a = a.nnz ();

  if (nr != nr_a || nc != nc_a || nz != nz_a)
    return false;

  for (octave_idx_type i = 0; i < nc + 1; i++)
    if (cidx(i) != a.cidx(i))
	return false;

  for (octave_idx_type i = 0; i < nz; i++)
    if (data(i) != a.data(i) || ridx(i) != a.ridx(i))
      return false;

  return true;
}

bool
SparseComplexMatrix::operator != (const SparseComplexMatrix& a) const
{
  return !(*this == a);
}

bool
SparseComplexMatrix::is_hermitian (void) const
{
  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();

  if (is_square () && nr > 0)
    {
      for (octave_idx_type i = 0; i < nr; i++)
	for (octave_idx_type j = i; j < nc; j++)
	  if (elem (i, j) != conj (elem (j, i)))
	    return false;

      return true;
    }

  return false;
}

static const Complex Complex_NaN_result (octave_NaN, octave_NaN);

SparseComplexMatrix
SparseComplexMatrix::max (int dim) const
{
  Array2<octave_idx_type> dummy_idx;
  return max (dummy_idx, dim);
}

SparseComplexMatrix
SparseComplexMatrix::max (Array2<octave_idx_type>& idx_arg, int dim) const
{
  SparseComplexMatrix result;
  dim_vector dv = dims ();

  if (dv.numel () == 0 || dim > dv.length () || dim < 0)
    return result;
 
  octave_idx_type nr = dv(0);
  octave_idx_type nc = dv(1);

  if (dim == 0)
    {
      idx_arg.resize (1, nc);
      octave_idx_type nel = 0;
      for (octave_idx_type j = 0; j < nc; j++)
	{
	  Complex tmp_max;
	  double abs_max = octave_NaN;
	  octave_idx_type idx_j = 0;
	  for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
	    {
	      if (ridx(i) != idx_j)
		break;
	      else
		idx_j++;
	    }

	  if (idx_j != nr)
	    {
	      tmp_max = 0.;
	      abs_max = 0.;
	    }

	  for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
	    {
	      Complex tmp = data (i);

	      if (xisnan (tmp))
		continue;

	      double abs_tmp = std::abs (tmp);

	      if (xisnan (abs_max) || abs_tmp > abs_max)
		{
		  idx_j = ridx (i);
		  tmp_max = tmp;
		  abs_max = abs_tmp;
		}
	    }

 	  idx_arg.elem (j) = xisnan (tmp_max) ? 0 : idx_j;
	  if (abs_max != 0.)
	    nel++;
	}

      result = SparseComplexMatrix (1, nc, nel);

      octave_idx_type ii = 0;
      result.xcidx (0) = 0;
      for (octave_idx_type j = 0; j < nc; j++)
	{
	  Complex tmp = elem (idx_arg(j), j);
	  if (tmp != 0.)
	    {
	      result.xdata (ii) = tmp;
	      result.xridx (ii++) = 0;
	    }
	  result.xcidx (j+1) = ii;
	}
    }
  else
    {
      idx_arg.resize (nr, 1, 0);

      for (octave_idx_type i = cidx(0); i < cidx(1); i++)
	idx_arg.elem(ridx(i)) = -1;

      for (octave_idx_type j = 0; j < nc; j++)
	for (octave_idx_type i = 0; i < nr; i++)
	  {
	    if (idx_arg.elem(i) != -1)
	      continue;
	    bool found = false;
	    for (octave_idx_type k = cidx(j); k < cidx(j+1); k++)
	      if (ridx(k) == i)
		{
		  found = true;
		  break;
		}
	    
	    if (!found)
	      idx_arg.elem(i) = j;

	  }

      for (octave_idx_type j = 0; j < nc; j++)
	{
	  for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
	    {
	      octave_idx_type ir = ridx (i);
	      octave_idx_type ix = idx_arg.elem (ir);
	      Complex tmp = data (i);

	      if (xisnan (tmp))
		continue;
	      else if (ix == -1 || std::abs(tmp) > std::abs(elem (ir, ix)))
		idx_arg.elem (ir) = j;
	    }
	}

      octave_idx_type nel = 0;
      for (octave_idx_type j = 0; j < nr; j++)
	if (idx_arg.elem(j) == -1 || elem (j, idx_arg.elem (j)) != 0.)
	  nel++;

      result = SparseComplexMatrix (nr, 1, nel);

      octave_idx_type ii = 0;
      result.xcidx (0) = 0;
      result.xcidx (1) = nel;
      for (octave_idx_type j = 0; j < nr; j++)
	{
	  if (idx_arg(j) == -1)
	    {
	      idx_arg(j) = 0;
	      result.xdata (ii) = Complex_NaN_result;
	      result.xridx (ii++) = j;
	    }
	  else
	    {
	      Complex tmp = elem (j, idx_arg(j));
	      if (tmp != 0.)
		{
		  result.xdata (ii) = tmp;
		  result.xridx (ii++) = j;
		}
	    }
	}
    }

  return result;
}

SparseComplexMatrix
SparseComplexMatrix::min (int dim) const
{
  Array2<octave_idx_type> dummy_idx;
  return min (dummy_idx, dim);
}

SparseComplexMatrix
SparseComplexMatrix::min (Array2<octave_idx_type>& idx_arg, int dim) const
{
  SparseComplexMatrix result;
  dim_vector dv = dims ();

  if (dv.numel () == 0 || dim > dv.length () || dim < 0)
    return result;
 
  octave_idx_type nr = dv(0);
  octave_idx_type nc = dv(1);

  if (dim == 0)
    {
      idx_arg.resize (1, nc);
      octave_idx_type nel = 0;
      for (octave_idx_type j = 0; j < nc; j++)
	{
	  Complex tmp_min;
	  double abs_min = octave_NaN;
	  octave_idx_type idx_j = 0;
	  for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
	    {
	      if (ridx(i) != idx_j)
		break;
	      else
		idx_j++;
	    }

	  if (idx_j != nr)
	    {
	      tmp_min = 0.;
	      abs_min = 0.;
	    }

	  for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
	    {
	      Complex tmp = data (i);

	      if (xisnan (tmp))
		continue;

	      double abs_tmp = std::abs (tmp);

	      if (xisnan (abs_min) || abs_tmp < abs_min)
		{
		  idx_j = ridx (i);
		  tmp_min = tmp;
		  abs_min = abs_tmp;
		}
	    }

 	  idx_arg.elem (j) = xisnan (tmp_min) ? 0 : idx_j;
	  if (abs_min != 0.)
	    nel++;
	}

      result = SparseComplexMatrix (1, nc, nel);

      octave_idx_type ii = 0;
      result.xcidx (0) = 0;
      for (octave_idx_type j = 0; j < nc; j++)
	{
	  Complex tmp = elem (idx_arg(j), j);
	  if (tmp != 0.)
	    {
	      result.xdata (ii) = tmp;
	      result.xridx (ii++) = 0;
	    }
	  result.xcidx (j+1) = ii;
	}
    }
  else
    {
      idx_arg.resize (nr, 1, 0);

      for (octave_idx_type i = cidx(0); i < cidx(1); i++)
	idx_arg.elem(ridx(i)) = -1;

      for (octave_idx_type j = 0; j < nc; j++)
	for (octave_idx_type i = 0; i < nr; i++)
	  {
	    if (idx_arg.elem(i) != -1)
	      continue;
	    bool found = false;
	    for (octave_idx_type k = cidx(j); k < cidx(j+1); k++)
	      if (ridx(k) == i)
		{
		  found = true;
		  break;
		}
	    
	    if (!found)
	      idx_arg.elem(i) = j;

	  }

      for (octave_idx_type j = 0; j < nc; j++)
	{
	  for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
	    {
	      octave_idx_type ir = ridx (i);
	      octave_idx_type ix = idx_arg.elem (ir);
	      Complex tmp = data (i);

	      if (xisnan (tmp))
		continue;
	      else if (ix == -1 || std::abs(tmp) < std::abs(elem (ir, ix)))
		idx_arg.elem (ir) = j;
	    }
	}

      octave_idx_type nel = 0;
      for (octave_idx_type j = 0; j < nr; j++)
	if (idx_arg.elem(j) == -1 || elem (j, idx_arg.elem (j)) != 0.)
	  nel++;

      result = SparseComplexMatrix (nr, 1, nel);

      octave_idx_type ii = 0;
      result.xcidx (0) = 0;
      result.xcidx (1) = nel;
      for (octave_idx_type j = 0; j < nr; j++)
	{
	  if (idx_arg(j) == -1)
	    {
	      idx_arg(j) = 0;
	      result.xdata (ii) = Complex_NaN_result;
	      result.xridx (ii++) = j;
	    }
	  else
	    {
	      Complex tmp = elem (j, idx_arg(j));
	      if (tmp != 0.)
		{
		  result.xdata (ii) = tmp;
		  result.xridx (ii++) = j;
		}
	    }
	}
    }

  return result;
}

// destructive insert/delete/reorder operations

SparseComplexMatrix&
SparseComplexMatrix::insert (const SparseMatrix& a, octave_idx_type r, octave_idx_type c)
{
  SparseComplexMatrix tmp (a);
  return insert (a, r, c);
}

SparseComplexMatrix&
SparseComplexMatrix::insert (const SparseComplexMatrix& a, octave_idx_type r, octave_idx_type c)
{
  MSparse<Complex>::insert (a, r, c);
  return *this;
}

SparseComplexMatrix
SparseComplexMatrix::concat (const SparseComplexMatrix& rb, 
			     const Array<octave_idx_type>& ra_idx)
{
  // Don't use numel to avoid all possiblity of an overflow
  if (rb.rows () > 0 && rb.cols () > 0)
    insert (rb, ra_idx(0), ra_idx(1));
  return *this;
}

SparseComplexMatrix
SparseComplexMatrix::concat (const SparseMatrix& rb, const Array<octave_idx_type>& ra_idx)
{
  SparseComplexMatrix tmp (rb);
  if (rb.rows () > 0 && rb.cols () > 0)
    insert (tmp, ra_idx(0), ra_idx(1));
  return *this;
}

ComplexMatrix
SparseComplexMatrix::matrix_value (void) const
{
  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  ComplexMatrix retval (nr, nc, Complex (0.0, 0.0));

  for (octave_idx_type j = 0; j < nc; j++)
    for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
      retval.elem (ridx(i), j) = data (i);

  return retval;
}

SparseComplexMatrix
SparseComplexMatrix::hermitian (void) const
{
  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  octave_idx_type nz = nnz ();
  SparseComplexMatrix retval (nc, nr, nz);

  retval.cidx(0) = 0;
  for (octave_idx_type i = 0, iidx = 0; i < nr; i++)
    {
      for (octave_idx_type j = 0; j < nc; j++)
	for (octave_idx_type k = cidx(j); k < cidx(j+1); k++)
	  if (ridx(k) == i)
	    {
	      retval.data(iidx) = conj (data(k));
	      retval.ridx(iidx++) = j;
	    }
      retval.cidx(i+1) = iidx;
    }

  return retval;
}

SparseComplexMatrix
conj (const SparseComplexMatrix& a)
{
  octave_idx_type nr = a.rows ();
  octave_idx_type nc = a.cols ();
  octave_idx_type nz = a.nnz ();
  SparseComplexMatrix retval (nc, nr, nz);

  for (octave_idx_type i = 0; i < nc + 1; i++)
    retval.cidx (i) = a.cidx (i);

  for (octave_idx_type i = 0; i < nz; i++)
    {
      retval.data (i) = conj (a.data (i));
      retval.ridx (i) = a.ridx (i);
    }

  return retval;
}

SparseComplexMatrix
SparseComplexMatrix::inverse (void) const
{
  octave_idx_type info;
  double rcond;
  SparseType mattype (*this);
  return inverse (mattype, info, rcond, 0, 0);
}

SparseComplexMatrix
SparseComplexMatrix::inverse (SparseType& mattype) const
{
  octave_idx_type info;
  double rcond;
  return inverse (mattype, info, rcond, 0, 0);
}

SparseComplexMatrix
SparseComplexMatrix::inverse (SparseType& mattype, octave_idx_type& info) const
{
  double rcond;
  return inverse (mattype, info, rcond, 0, 0);
}

SparseComplexMatrix 
SparseComplexMatrix::dinverse (SparseType &mattyp, octave_idx_type& info, 
			double& rcond, const bool force,
			const bool calccond) const
{
  SparseComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  info = 0;

  if (nr == 0 || nc == 0 || nr != nc)
    (*current_liboctave_error_handler) ("inverse requires square matrix");
  else
    {
      // Print spparms("spumoni") info if requested
      int typ = mattyp.type ();
      mattyp.info ();

      if (typ == SparseType::Diagonal ||
	  typ == SparseType::Permuted_Diagonal)
	{
	  if (typ == SparseType::Permuted_Diagonal)
	    retval = transpose();
	  else
	    retval = *this;
	      
	  // Force make_unique to be called
	  Complex *v = retval.data();

	  if (calccond)
	    {
	      double dmax = 0., dmin = octave_Inf; 
	      for (octave_idx_type i = 0; i < nr; i++)
		{
		  double tmp = std::abs(v[i]);
		  if (tmp > dmax)
		    dmax = tmp;
		  if (tmp < dmin)
		    dmin = tmp;
		}
	      rcond = dmin / dmax;
	    }

	  for (octave_idx_type i = 0; i < nr; i++)
	    v[i] = 1.0 / v[i];
	}
      else
	(*current_liboctave_error_handler) ("incorrect matrix type");
    }

  return retval;
}

SparseComplexMatrix 
SparseComplexMatrix::tinverse (SparseType &mattyp, octave_idx_type& info, 
			       double& rcond, const bool force, 
			       const bool calccond) const
{
  SparseComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  info = 0;

  if (nr == 0 || nc == 0 || nr != nc)
    (*current_liboctave_error_handler) ("inverse requires square matrix");
  else
    {
      // Print spparms("spumoni") info if requested
      int typ = mattyp.type ();
      mattyp.info ();

      if (typ == SparseType::Upper || typ == SparseType::Permuted_Upper ||
	  typ == SparseType::Lower || typ == SparseType::Permuted_Lower)
	{
	  double anorm = 0.;
	  double ainvnorm = 0.;

	  if (calccond)
	    {
	      // Calculate the 1-norm of matrix for rcond calculation
	      for (octave_idx_type j = 0; j < nr; j++)
		{
		  double atmp = 0.;
		  for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
		    atmp += std::abs(data(i));
		  if (atmp > anorm)
		    anorm = atmp;
		}
	    }

	  if (typ == SparseType::Upper || typ == SparseType::Lower)
	    {
	      octave_idx_type nz = nnz();
	      octave_idx_type cx = 0;
	      octave_idx_type nz2 = nz;
	      retval = SparseComplexMatrix (nr, nc, nz2);

	      for (octave_idx_type i = 0; i < nr; i++)
		{
		  OCTAVE_QUIT;
		  // place the 1 in the identity position
		  octave_idx_type cx_colstart = cx;
	  
		  if (cx == nz2)
		    {
		      nz2 *= 2;
		      retval.change_capacity (nz2);
		    }

		  retval.xcidx(i) = cx;
		  retval.xridx(cx) = i;
		  retval.xdata(cx) = 1.0;
		  cx++;

		  // iterate accross columns of input matrix
		  for (octave_idx_type j = i+1; j < nr; j++) 
		    {
		      Complex v = 0.;
		      // iterate to calculate sum
		      octave_idx_type colXp = retval.xcidx(i);
		      octave_idx_type colUp = cidx(j);
		      octave_idx_type rpX, rpU;
		      do
			{
			  OCTAVE_QUIT;
			  rpX = retval.xridx(colXp);
			  rpU = ridx(colUp);

			  if (rpX < rpU) 
			    colXp++;
			  else if (rpX > rpU) 
			    colUp++;
			  else 
			    {
			      v -= retval.xdata(colXp) * data(colUp);
			      colXp++;
			      colUp++;
			    }
			} while ((rpX<j) && (rpU<j) && 
				 (colXp<cx) && (colUp<nz));

		      // get A(m,m)
		      colUp = cidx(j+1) - 1;
		      Complex pivot = data(colUp);
		      if (pivot == 0.) 
			(*current_liboctave_error_handler) 
			  ("division by zero");

		      if (v != 0.)
			{
			  if (cx == nz2)
			    {
			      nz2 *= 2;
			      retval.change_capacity (nz2);
			    }

			  retval.xridx(cx) = j;
			  retval.xdata(cx) = v / pivot;
			  cx++;
			}
		    }

		  // get A(m,m)
		  octave_idx_type colUp = cidx(i+1) - 1;
		  Complex pivot = data(colUp);
		  if (pivot == 0.) 
		    (*current_liboctave_error_handler) ("division by zero");

		  if (pivot != 1.0)
		    for (octave_idx_type j = cx_colstart; j < cx; j++)
		      retval.xdata(j) /= pivot;
		}
	      retval.xcidx(nr) = cx;
	      retval.maybe_compress ();
	    }
	  else
	    {
	      octave_idx_type nz = nnz();
	      octave_idx_type cx = 0;
	      octave_idx_type nz2 = nz;
	      retval = SparseComplexMatrix (nr, nc, nz2);

	      OCTAVE_LOCAL_BUFFER (Complex, work, nr);
	      OCTAVE_LOCAL_BUFFER (octave_idx_type, rperm, nr);

	      octave_idx_type *perm = mattyp.triangular_perm();
	      if (typ == SparseType::Permuted_Upper)
		{
		  for (octave_idx_type i = 0; i < nr; i++)
		    rperm[perm[i]] = i;
		}
	      else
		{
		  for (octave_idx_type i = 0; i < nr; i++)
		    rperm[i] = perm[i];
		  for (octave_idx_type i = 0; i < nr; i++)
		    perm[rperm[i]] = i;
		}

	      for (octave_idx_type i = 0; i < nr; i++)
		{
		  OCTAVE_QUIT;
		  octave_idx_type iidx = rperm[i];

		  for (octave_idx_type j = 0; j < nr; j++)
		    work[j] = 0.;

		  // place the 1 in the identity position
		  work[iidx] = 1.0;

		  // iterate accross columns of input matrix
		  for (octave_idx_type j = iidx+1; j < nr; j++) 
		    {
		      Complex v = 0.;
		      octave_idx_type jidx = perm[j];
		      // iterate to calculate sum
		      for (octave_idx_type k = cidx(jidx); 
			   k < cidx(jidx+1); k++)
			{
			  OCTAVE_QUIT;
			  v -= work[ridx(k)] * data(k);
			}

		      // get A(m,m)
		      Complex pivot = data(cidx(jidx+1) - 1);
		      if (pivot == 0.) 
			(*current_liboctave_error_handler) 
			  ("division by zero");

		      work[j] = v / pivot;
		    }

		  // get A(m,m)
		  octave_idx_type colUp = cidx(perm[iidx]+1) - 1;
		  Complex pivot = data(colUp);
		  if (pivot == 0.) 
		    (*current_liboctave_error_handler) 
		      ("division by zero");

		  octave_idx_type new_cx = cx;
		  for (octave_idx_type j = iidx; j < nr; j++)
		    if (work[j] != 0.0)
		      {
			new_cx++;
			if (pivot != 1.0)
			  work[j] /= pivot;
		      }

		  if (cx < new_cx)
		    {
		      nz2 = (2*nz2 < new_cx ? new_cx : 2*nz2);
		      retval.change_capacity (nz2);
		    }

		  retval.xcidx(i) = cx;
		  for (octave_idx_type j = iidx; j < nr; j++)
		    if (work[j] != 0.)
		      {
			retval.xridx(cx) = j;
			retval.xdata(cx++) = work[j];
		      }
		}

	      retval.xcidx(nr) = cx;
	      retval.maybe_compress ();
	    }

	  if (calccond)
	    {
	      // Calculate the 1-norm of inverse matrix for rcond calculation
	      for (octave_idx_type j = 0; j < nr; j++)
		{
		  double atmp = 0.;
		  for (octave_idx_type i = retval.cidx(j); 
		       i < retval.cidx(j+1); i++)
		    atmp += std::abs(retval.data(i));
		  if (atmp > ainvnorm)
		    ainvnorm = atmp;
		}

	      rcond = 1. / ainvnorm / anorm;     
	    }
	}
      else
	(*current_liboctave_error_handler) ("incorrect matrix type");
    }

  return retval;
}

SparseComplexMatrix
SparseComplexMatrix::inverse (SparseType& mattype, octave_idx_type& info, 
			      double& rcond, int force, int calc_cond) const
{
  int typ = mattype.type (false);
  SparseComplexMatrix ret;

  if (typ == SparseType::Unknown)
    typ = mattype.type (*this);

  if (typ == SparseType::Diagonal || typ == SparseType::Permuted_Diagonal)
    ret = dinverse (mattype, info, rcond, true, calc_cond);
  else if (typ == SparseType::Upper || typ == SparseType::Permuted_Upper)
    ret = tinverse (mattype, info, rcond, true, calc_cond).transpose();
  else if (typ == SparseType::Lower || typ == SparseType::Permuted_Lower)
    ret = transpose().tinverse (mattype, info, rcond, true, calc_cond);
  else if (typ != SparseType::Rectangular)
    {
      if (mattype.is_hermitian())
	{
	  SparseType tmp_typ (SparseType::Upper);
	  SparseComplexCHOL fact (*this, info, false);
	  rcond = fact.rcond();
	  if (info == 0)
	    {
	      double rcond2;
	      SparseMatrix Q = fact.Q();
	      SparseComplexMatrix InvL = fact.L().transpose().
		tinverse(tmp_typ, info, rcond2, true, false);
	      ret = Q * InvL.hermitian() * InvL * Q.transpose();
	    }
	  else
	    {
	      // Matrix is either singular or not positive definite
	      mattype.mark_as_unsymmetric ();
	      typ = SparseType::Full;
	    }
	}

      if (!mattype.is_hermitian())
	{
	  octave_idx_type n = rows();
	  ColumnVector Qinit(n);
	  for (octave_idx_type i = 0; i < n; i++)
	    Qinit(i) = i;

	  SparseType tmp_typ (SparseType::Upper);
	  SparseComplexLU fact (*this, Qinit, -1.0, false);
	  rcond = fact.rcond();
	  double rcond2;
	  SparseComplexMatrix InvL = fact.L().transpose().
	    tinverse(tmp_typ, info, rcond2, true, false);
	  SparseComplexMatrix InvU = fact.U().
	    tinverse(tmp_typ, info, rcond2, true, false).transpose();
	  ret = fact.Pc().transpose() * InvU * InvL * fact.Pr();
	}
    }
  else
    (*current_liboctave_error_handler) ("inverse requires square matrix");

  return ret;
}

ComplexDET
SparseComplexMatrix::determinant (void) const
{
  octave_idx_type info;
  double rcond;
  return determinant (info, rcond, 0);
}

ComplexDET
SparseComplexMatrix::determinant (octave_idx_type& info) const
{
  double rcond;
  return determinant (info, rcond, 0);
}

ComplexDET
SparseComplexMatrix::determinant (octave_idx_type& err, double& rcond, int calc_cond) const
{
  ComplexDET retval;
#ifdef HAVE_UMFPACK

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();

  if (nr == 0 || nc == 0 || nr != nc)
    {
      Complex d[2];
      d[0] = 1.0;
      d[1] = 0.0;
      retval = ComplexDET (d);
    }
  else
    {
      err = 0;

      // Setup the control parameters
      Matrix Control (UMFPACK_CONTROL, 1);
      double *control = Control.fortran_vec ();
      UMFPACK_ZNAME (defaults) (control);

      double tmp = Voctave_sparse_controls.get_key ("spumoni");
      if (!xisnan (tmp))
	Control (UMFPACK_PRL) = tmp;

      tmp = Voctave_sparse_controls.get_key ("piv_tol");
      if (!xisnan (tmp))
	{
	  Control (UMFPACK_SYM_PIVOT_TOLERANCE) = tmp;
	  Control (UMFPACK_PIVOT_TOLERANCE) = tmp;
	}

      // Set whether we are allowed to modify Q or not
      tmp = Voctave_sparse_controls.get_key ("autoamd");
      if (!xisnan (tmp))
	Control (UMFPACK_FIXQ) = tmp;

      // Turn-off UMFPACK scaling for LU 
      Control (UMFPACK_SCALE) = UMFPACK_SCALE_NONE;

      UMFPACK_ZNAME (report_control) (control);

      const octave_idx_type *Ap = cidx ();
      const octave_idx_type *Ai = ridx ();
      const Complex *Ax = data ();

      UMFPACK_ZNAME (report_matrix) (nr, nc, Ap, Ai, 
				    X_CAST (const double *, Ax), 
				    NULL, 1, control);

      void *Symbolic;
      Matrix Info (1, UMFPACK_INFO);
      double *info = Info.fortran_vec ();
      int status = UMFPACK_ZNAME (qsymbolic) 
	(nr, nc, Ap, Ai, X_CAST (const double *, Ax), NULL, 
	 NULL, &Symbolic, control, info);

      if (status < 0)
	{
	  (*current_liboctave_error_handler) 
	    ("SparseComplexMatrix::determinant symbolic factorization failed");

	  UMFPACK_ZNAME (report_status) (control, status);
	  UMFPACK_ZNAME (report_info) (control, info);

	  UMFPACK_ZNAME (free_symbolic) (&Symbolic) ;
	}
      else
	{
	  UMFPACK_ZNAME (report_symbolic) (Symbolic, control);

	  void *Numeric;
	  status = UMFPACK_ZNAME (numeric) (Ap, Ai,
				       X_CAST (const double *, Ax), NULL,
				       Symbolic, &Numeric, control, info) ;
	  UMFPACK_ZNAME (free_symbolic) (&Symbolic) ;

	  rcond = Info (UMFPACK_RCOND);

	  if (status < 0)
	    {
	      (*current_liboctave_error_handler) 
		("SparseComplexMatrix::determinant numeric factorization failed");

	      UMFPACK_ZNAME (report_status) (control, status);
	      UMFPACK_ZNAME (report_info) (control, info);

	      UMFPACK_ZNAME (free_numeric) (&Numeric);
	    }
	  else
	    {
	      UMFPACK_ZNAME (report_numeric) (Numeric, control);

	      Complex d[2];
	      double d_exponent;

	      status = UMFPACK_ZNAME (get_determinant) 
		(X_CAST (double *, &d[0]), NULL, &d_exponent,
		 Numeric, info);
	      d[1] = d_exponent;

	      if (status < 0)
		{
		  (*current_liboctave_error_handler) 
		    ("SparseComplexMatrix::determinant error calculating determinant");
		  
		  UMFPACK_ZNAME (report_status) (control, status);
		  UMFPACK_ZNAME (report_info) (control, info);
		}
	      else
		retval = ComplexDET (d);
		  
	      UMFPACK_ZNAME (free_numeric) (&Numeric);
	    }
	}
    }
#else
  (*current_liboctave_error_handler) ("UMFPACK not installed");
#endif

  return retval;
}

ComplexMatrix
SparseComplexMatrix::dsolve (SparseType &mattype, const Matrix& b, octave_idx_type& err, 
			     double& rcond, solve_singularity_handler) const
{
  ComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  err = 0;

  if (nr == 0 || nc == 0 || nr != nc || nr != b.rows ())
    (*current_liboctave_error_handler)
      ("matrix dimension mismatch solution of linear equations");
  else
    {
      // Print spparms("spumoni") info if requested
      int typ = mattype.type ();
      mattype.info ();

      if (typ == SparseType::Diagonal ||
	  typ == SparseType::Permuted_Diagonal)
	{
	  retval.resize (b.rows (), b.cols());
	  if (typ == SparseType::Diagonal)
	    for (octave_idx_type j = 0; j < b.cols(); j++)
	      for (octave_idx_type i = 0; i < nr; i++)
		retval(i,j) = b(i,j) / data (i);
	  else
	    for (octave_idx_type j = 0; j < b.cols(); j++)
	      for (octave_idx_type i = 0; i < nr; i++)
		retval(i,j) = b(ridx(i),j) / data (i);
	    
	  double dmax = 0., dmin = octave_Inf; 
	  for (octave_idx_type i = 0; i < nr; i++)
	    {
	      double tmp = std::abs(data(i));
	      if (tmp > dmax)
		dmax = tmp;
	      if (tmp < dmin)
		dmin = tmp;
	    }
	  rcond = dmin / dmax;
	}
      else
	(*current_liboctave_error_handler) ("incorrect matrix type");
    }

  return retval;
}

SparseComplexMatrix
SparseComplexMatrix::dsolve (SparseType &mattype, const SparseMatrix& b,
		       octave_idx_type& err, double& rcond, solve_singularity_handler) const
{
  SparseComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  err = 0;

  if (nr == 0 || nc == 0 || nr != nc || nr != b.rows ())
    (*current_liboctave_error_handler)
      ("matrix dimension mismatch solution of linear equations");
  else
    {
      // Print spparms("spumoni") info if requested
      int typ = mattype.type ();
      mattype.info ();

      if (typ == SparseType::Diagonal ||
	  typ == SparseType::Permuted_Diagonal)
	{
	  octave_idx_type b_nr = b.rows ();
	  octave_idx_type b_nc = b.cols ();
	  octave_idx_type b_nz = b.nnz ();
	  retval = SparseComplexMatrix (b_nr, b_nc, b_nz);

	  retval.xcidx(0) = 0;
	  octave_idx_type ii = 0;
	  if (typ == SparseType::Diagonal)
	    for (octave_idx_type j = 0; j < b.cols(); j++)
	      {
		for (octave_idx_type i = b.cidx(j); i < b.cidx(j+1); i++)
		  {
		    retval.xridx (ii) = b.ridx(i);
		    retval.xdata (ii++) = b.data(i) / data (b.ridx (i));
		  }
		retval.xcidx(j+1) = ii;
	      }
	  else
	    for (octave_idx_type j = 0; j < b.cols(); j++)
	      {
		for (octave_idx_type i = 0; i < nr; i++)
		  {
		    bool found = false;
		    octave_idx_type k;
		    for (k = b.cidx(j); k < b.cidx(j+1); k++)
		      if (ridx(i) == b.ridx(k))
			{
			  found = true;
			  break;
			}
		    if (found)
		      {
			retval.xridx (ii) = i;
			retval.xdata (ii++) = b.data(k) / data (i);
		      }
		  }
		retval.xcidx(j+1) = ii;
	      }
	    
	  double dmax = 0., dmin = octave_Inf; 
	  for (octave_idx_type i = 0; i < nr; i++)
	    {
	      double tmp = std::abs(data(i));
	      if (tmp > dmax)
		dmax = tmp;
	      if (tmp < dmin)
		dmin = tmp;
	    }
	  rcond = dmin / dmax;
	}
      else
	(*current_liboctave_error_handler) ("incorrect matrix type");
    }

  return retval;
}

ComplexMatrix
SparseComplexMatrix::dsolve (SparseType &mattype, const ComplexMatrix& b,
		     octave_idx_type& err, double& rcond, solve_singularity_handler) const
{
  ComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  err = 0;

  if (nr == 0 || nc == 0 || nr != nc || nr != b.rows ())
    (*current_liboctave_error_handler)
      ("matrix dimension mismatch solution of linear equations");
  else
    {
      // Print spparms("spumoni") info if requested
      int typ = mattype.type ();
      mattype.info ();

      if (typ == SparseType::Diagonal ||
	  typ == SparseType::Permuted_Diagonal)
	{
	  retval.resize (b.rows (), b.cols());
	  if (typ == SparseType::Diagonal)
	    for (octave_idx_type j = 0; j < b.cols(); j++)
	      for (octave_idx_type i = 0; i < nr; i++)
		retval(i,j) = b(i,j) / data (i);
	  else
	    for (octave_idx_type j = 0; j < b.cols(); j++)
	      for (octave_idx_type i = 0; i < nr; i++)
		retval(i,j) = b(ridx(i),j) / data (i);
	    
	  double dmax = 0., dmin = octave_Inf; 
	  for (octave_idx_type i = 0; i < nr; i++)
	    {
	      double tmp = std::abs(data(i));
	      if (tmp > dmax)
		dmax = tmp;
	      if (tmp < dmin)
		dmin = tmp;
	    }
	  rcond = dmin / dmax;
	}
      else
	(*current_liboctave_error_handler) ("incorrect matrix type");
    }

  return retval;
}

SparseComplexMatrix
SparseComplexMatrix::dsolve (SparseType &mattype, const SparseComplexMatrix& b,
		     octave_idx_type& err, double& rcond, 
		     solve_singularity_handler) const
{
  SparseComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  err = 0;

  if (nr == 0 || nc == 0 || nr != nc || nr != b.rows ())
    (*current_liboctave_error_handler)
      ("matrix dimension mismatch solution of linear equations");
  else
    {
      // Print spparms("spumoni") info if requested
      int typ = mattype.type ();
      mattype.info ();

      if (typ == SparseType::Diagonal ||
	  typ == SparseType::Permuted_Diagonal)
	{
	  octave_idx_type b_nr = b.rows ();
	  octave_idx_type b_nc = b.cols ();
	  octave_idx_type b_nz = b.nnz ();
	  retval = SparseComplexMatrix (b_nr, b_nc, b_nz);

	  retval.xcidx(0) = 0;
	  octave_idx_type ii = 0;
	  if (typ == SparseType::Diagonal)
	    for (octave_idx_type j = 0; j < b.cols(); j++)
	      {
		for (octave_idx_type i = b.cidx(j); i < b.cidx(j+1); i++)
		  {
		    retval.xridx (ii) = b.ridx(i);
		    retval.xdata (ii++) = b.data(i) / data (b.ridx (i));
		  }
		retval.xcidx(j+1) = ii;
	      }
	  else
	    for (octave_idx_type j = 0; j < b.cols(); j++)
	      {
		for (octave_idx_type i = 0; i < nr; i++)
		  {
		    bool found = false;
		    octave_idx_type k;
		    for (k = b.cidx(j); k < b.cidx(j+1); k++)
		      if (ridx(i) == b.ridx(k))
			{
			  found = true;
			  break;
			}
		    if (found)
		      {
			retval.xridx (ii) = i;
			retval.xdata (ii++) = b.data(k) / data (i);
		      }
		  }
		retval.xcidx(j+1) = ii;
	      }
	    
	  double dmax = 0., dmin = octave_Inf; 
	  for (octave_idx_type i = 0; i < nr; i++)
	    {
	      double tmp = std::abs(data(i));
	      if (tmp > dmax)
		dmax = tmp;
	      if (tmp < dmin)
		dmin = tmp;
	    }
	  rcond = dmin / dmax;
	}
      else
	(*current_liboctave_error_handler) ("incorrect matrix type");
    }

  return retval;
}

ComplexMatrix
SparseComplexMatrix::utsolve (SparseType &mattype, const Matrix& b, octave_idx_type& err, 
		       double& rcond,
		       solve_singularity_handler sing_handler) const
{
  ComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  err = 0;

  if (nr == 0 || nc == 0 || nr != nc || nr != b.rows ())
    (*current_liboctave_error_handler)
      ("matrix dimension mismatch solution of linear equations");
  else
    {
      // Print spparms("spumoni") info if requested
      int typ = mattype.type ();
      mattype.info ();

      if (typ == SparseType::Permuted_Upper ||
	  typ == SparseType::Upper)
	{
	  double anorm = 0.;
	  double ainvnorm = 0.;
	  octave_idx_type b_cols = b.cols ();
	  rcond = 0.;

	  // Calculate the 1-norm of matrix for rcond calculation
	  for (octave_idx_type j = 0; j < nr; j++)
	    {
	      double atmp = 0.;
	      for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
		atmp += std::abs(data(i));
	      if (atmp > anorm)
		anorm = atmp;
	    }

	  if (typ == SparseType::Permuted_Upper)
	    {
	      retval.resize (nr, b_cols);
	      octave_idx_type *perm = mattype.triangular_perm ();
	      OCTAVE_LOCAL_BUFFER (Complex, work, nr);

	      for (octave_idx_type j = 0; j < b_cols; j++)
		{
		  for (octave_idx_type i = 0; i < nr; i++)
		    work[i] = b(i,j);

		  for (octave_idx_type k = nr-1; k >= 0; k--)
		    {
		      octave_idx_type kidx = perm[k];

		      if (work[k] != 0.)
			{
			  if (ridx(cidx(kidx+1)-1) != k)
			    {
			      err = -2;
			      goto triangular_error;
			    }			    

			  Complex tmp = work[k] / data(cidx(kidx+1)-1);
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(kidx); 
			       i < cidx(kidx+1)-1; i++)
			    {
			      octave_idx_type iidx = ridx(i);
			      work[iidx] = work[iidx] - tmp * data(i);
			    }
			}
		    }

		  for (octave_idx_type i = 0; i < nr; i++)
		    retval (perm[i], j) = work[i];
		}

	      // Calculation of 1-norm of inv(*this)
	      for (octave_idx_type i = 0; i < nr; i++)
		work[i] = 0.;

	      for (octave_idx_type j = 0; j < nr; j++)
		{
		  work[j] = 1.;

		  for (octave_idx_type k = j; k >= 0; k--)
		    {
		      octave_idx_type iidx = perm[k];

		      if (work[k] != 0.)
			{
			  Complex tmp = work[k] / data(cidx(iidx+1)-1);
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(iidx); 
			       i < cidx(iidx+1)-1; i++)
			    {
			      octave_idx_type idx2 = ridx(i);
			      work[idx2] = work[idx2] - tmp * data(i);
			    }
			}
		    }
		  double atmp = 0;
		  for (octave_idx_type i = 0; i < j+1; i++)
		    {
		      atmp += std::abs(work[i]);
		      work[i] = 0.;
		    }
		  if (atmp > ainvnorm)
		    ainvnorm = atmp;
		}
	    }
	  else
	    {
	      retval = ComplexMatrix (b);
	      Complex *x_vec = retval.fortran_vec ();

	      for (octave_idx_type j = 0; j < b_cols; j++)
		{
		  octave_idx_type offset = j * nr;
		  for (octave_idx_type k = nr-1; k >= 0; k--)
		    {
		      if (x_vec[k+offset] != 0.)
			{
			  if (ridx(cidx(k+1)-1) != k)
			    {
			      err = -2;
			      goto triangular_error;
			    }			    

			  Complex tmp = x_vec[k+offset] / 
			    data(cidx(k+1)-1);
			  x_vec[k+offset] = tmp;
			  for (octave_idx_type i = cidx(k); i < cidx(k+1)-1; i++)
			    {
			      octave_idx_type iidx = ridx(i);
			      x_vec[iidx+offset] = 
				x_vec[iidx+offset] - tmp * data(i);
			    }
			}
		    }
		}

	      // Calculation of 1-norm of inv(*this)
	      OCTAVE_LOCAL_BUFFER (Complex, work, nr);
	      for (octave_idx_type i = 0; i < nr; i++)
		work[i] = 0.;

	      for (octave_idx_type j = 0; j < nr; j++)
		{
		  work[j] = 1.;

		  for (octave_idx_type k = j; k >= 0; k--)
		    {
		      if (work[k] != 0.)
			{
			  Complex tmp = work[k] / data(cidx(k+1)-1);
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(k); i < cidx(k+1)-1; i++)
			    {
			      octave_idx_type iidx = ridx(i);
			      work[iidx] = work[iidx] - tmp * data(i);
			    }
			}
		    }
		  double atmp = 0;
		  for (octave_idx_type i = 0; i < j+1; i++)
		    {
		      atmp += std::abs(work[i]);
		      work[i] = 0.;
		    }
		  if (atmp > ainvnorm)
		    ainvnorm = atmp;
		}
	    }

	  rcond = 1. / ainvnorm / anorm;

	triangular_error:
	  if (err != 0)
	    {
	      if (sing_handler)
		sing_handler (rcond);
	      else
		(*current_liboctave_error_handler)
		  ("SparseComplexMatrix::solve matrix singular to machine precision, rcond = %g",
		   rcond);
	    }

	  volatile double rcond_plus_one = rcond + 1.0;

	  if (rcond_plus_one == 1.0 || xisnan (rcond))
	    {
	      err = -2;

	      if (sing_handler)
		sing_handler (rcond);
	      else
		(*current_liboctave_error_handler)
		  ("matrix singular to machine precision, rcond = %g",
		   rcond);
	    }
	}
      else
	(*current_liboctave_error_handler) ("incorrect matrix type");
    }

  return retval;
}

SparseComplexMatrix
SparseComplexMatrix::utsolve (SparseType &mattype, const SparseMatrix& b,
			octave_idx_type& err, double& rcond, 
			solve_singularity_handler sing_handler) const
{
  SparseComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  err = 0;

  if (nr == 0 || nc == 0 || nr != nc || nr != b.rows ())
    (*current_liboctave_error_handler)
      ("matrix dimension mismatch solution of linear equations");
  else
    {
      // Print spparms("spumoni") info if requested
      int typ = mattype.type ();
      mattype.info ();

      if (typ == SparseType::Permuted_Upper ||
	  typ == SparseType::Upper)
	{
	  double anorm = 0.;
	  double ainvnorm = 0.;
	  rcond = 0.;

	  // Calculate the 1-norm of matrix for rcond calculation
	  for (octave_idx_type j = 0; j < nr; j++)
	    {
	      double atmp = 0.;
	      for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
		atmp += std::abs(data(i));
	      if (atmp > anorm)
		anorm = atmp;
	    }

	  octave_idx_type b_nr = b.rows ();
	  octave_idx_type b_nc = b.cols ();
	  octave_idx_type b_nz = b.nnz ();
	  retval = SparseComplexMatrix (b_nr, b_nc, b_nz);
	  retval.xcidx(0) = 0;
	  octave_idx_type ii = 0;
	  octave_idx_type x_nz = b_nz;

	  if (typ == SparseType::Permuted_Upper)
	    {
	      octave_idx_type *perm = mattype.triangular_perm ();
	      OCTAVE_LOCAL_BUFFER (Complex, work, nr);

	      OCTAVE_LOCAL_BUFFER (octave_idx_type, rperm, nr);
	      for (octave_idx_type i = 0; i < nr; i++)
		rperm[perm[i]] = i;

	      for (octave_idx_type j = 0; j < b_nc; j++)
		{
		  for (octave_idx_type i = 0; i < nr; i++)
		    work[i] = 0.;
		  for (octave_idx_type i = b.cidx(j); i < b.cidx(j+1); i++)
		    work[b.ridx(i)] = b.data(i);

		  for (octave_idx_type k = nr-1; k >= 0; k--)
		    {
		      octave_idx_type kidx = perm[k];

		      if (work[k] != 0.)
			{
			  if (ridx(cidx(kidx+1)-1) != k)
			    {
			      err = -2;
			      goto triangular_error;
			    }			    

			  Complex tmp = work[k] / data(cidx(kidx+1)-1);
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(kidx); 
			       i < cidx(kidx+1)-1; i++)
			    {
			      octave_idx_type iidx = ridx(i);
			      work[iidx] = work[iidx] - tmp * data(i);
			    }
			}
		    }

		  // Count non-zeros in work vector and adjust space in
		  // retval if needed
		  octave_idx_type new_nnz = 0;
		  for (octave_idx_type i = 0; i < nr; i++)
		    if (work[i] != 0.)
		      new_nnz++;

		  if (ii + new_nnz > x_nz)
		    {
		      // Resize the sparse matrix
		      octave_idx_type sz = new_nnz * (b_nc - j) + x_nz;
		      retval.change_capacity (sz);
		      x_nz = sz;
		    }

		  for (octave_idx_type i = 0; i < nr; i++)
		    if (work[rperm[i]] != 0.)
		      {
			retval.xridx(ii) = i;
			retval.xdata(ii++) = work[rperm[i]];
		      }
		  retval.xcidx(j+1) = ii;
		}

	      retval.maybe_compress ();

	      // Calculation of 1-norm of inv(*this)
	      for (octave_idx_type i = 0; i < nr; i++)
		work[i] = 0.;

	      for (octave_idx_type j = 0; j < nr; j++)
		{
		  work[j] = 1.;

		  for (octave_idx_type k = j; k >= 0; k--)
		    {
		      octave_idx_type iidx = perm[k];

		      if (work[k] != 0.)
			{
			  Complex tmp = work[k] / data(cidx(iidx+1)-1);
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(iidx); 
			       i < cidx(iidx+1)-1; i++)
			    {
			      octave_idx_type idx2 = ridx(i);
			      work[idx2] = work[idx2] - tmp * data(i);
			    }
			}
		    }
		  double atmp = 0;
		  for (octave_idx_type i = 0; i < j+1; i++)
		    {
		      atmp += std::abs(work[i]);
		      work[i] = 0.;
		    }
		  if (atmp > ainvnorm)
		    ainvnorm = atmp;
		}
	    }
	  else
	    {
	      OCTAVE_LOCAL_BUFFER (Complex, work, nr);

	      for (octave_idx_type j = 0; j < b_nc; j++)
		{
		  for (octave_idx_type i = 0; i < nr; i++)
		    work[i] = 0.;
		  for (octave_idx_type i = b.cidx(j); i < b.cidx(j+1); i++)
		    work[b.ridx(i)] = b.data(i);

		  for (octave_idx_type k = nr-1; k >= 0; k--)
		    {
		      if (work[k] != 0.)
			{
			  if (ridx(cidx(k+1)-1) != k)
			    {
			      err = -2;
			      goto triangular_error;
			    }			    

			  Complex tmp = work[k] / data(cidx(k+1)-1);
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(k); i < cidx(k+1)-1; i++)
			    {
			      octave_idx_type iidx = ridx(i);
			      work[iidx] = work[iidx] - tmp * data(i);
			    }
			}
		    }

		  // Count non-zeros in work vector and adjust space in
		  // retval if needed
		  octave_idx_type new_nnz = 0;
		  for (octave_idx_type i = 0; i < nr; i++)
		    if (work[i] != 0.)
		      new_nnz++;

		  if (ii + new_nnz > x_nz)
		    {
		      // Resize the sparse matrix
		      octave_idx_type sz = new_nnz * (b_nc - j) + x_nz;
		      retval.change_capacity (sz);
		      x_nz = sz;
		    }

		  for (octave_idx_type i = 0; i < nr; i++)
		    if (work[i] != 0.)
		      {
			retval.xridx(ii) = i;
			retval.xdata(ii++) = work[i];
		      }
		  retval.xcidx(j+1) = ii;
		}

	      retval.maybe_compress ();

	      // Calculation of 1-norm of inv(*this)
	      for (octave_idx_type i = 0; i < nr; i++)
		work[i] = 0.;

	      for (octave_idx_type j = 0; j < nr; j++)
		{
		  work[j] = 1.;

		  for (octave_idx_type k = j; k >= 0; k--)
		    {
		      if (work[k] != 0.)
			{
			  Complex tmp = work[k] / data(cidx(k+1)-1);
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(k); i < cidx(k+1)-1; i++)
			    {
			      octave_idx_type iidx = ridx(i);
			      work[iidx] = work[iidx] - tmp * data(i);
			    }
			}
		    }
		  double atmp = 0;
		  for (octave_idx_type i = 0; i < j+1; i++)
		    {
		      atmp += std::abs(work[i]);
		      work[i] = 0.;
		    }
		  if (atmp > ainvnorm)
		    ainvnorm = atmp;
		}
	    }

	  rcond = 1. / ainvnorm / anorm;

	triangular_error:
	  if (err != 0)
	    {
	      if (sing_handler)
		sing_handler (rcond);
	      else
		(*current_liboctave_error_handler)
		  ("SparseComplexMatrix::solve matrix singular to machine precision, rcond = %g",
		   rcond);
	    }

	  volatile double rcond_plus_one = rcond + 1.0;

	  if (rcond_plus_one == 1.0 || xisnan (rcond))
	    {
	      err = -2;

	      if (sing_handler)
		sing_handler (rcond);
	      else
		(*current_liboctave_error_handler)
		  ("matrix singular to machine precision, rcond = %g",
		   rcond);
	    }
	}
      else
	(*current_liboctave_error_handler) ("incorrect matrix type");
    }
  return retval;
}

ComplexMatrix
SparseComplexMatrix::utsolve (SparseType &mattype, const ComplexMatrix& b,
		     octave_idx_type& err, double& rcond, 
		     solve_singularity_handler sing_handler) const
{
  ComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  err = 0;

  if (nr == 0 || nc == 0 || nr != nc || nr != b.rows ())
    (*current_liboctave_error_handler)
      ("matrix dimension mismatch solution of linear equations");
  else
    {
      // Print spparms("spumoni") info if requested
      int typ = mattype.type ();
      mattype.info ();
      
      if (typ == SparseType::Permuted_Upper ||
	  typ == SparseType::Upper)
	{
	  double anorm = 0.;
	  double ainvnorm = 0.;
	  octave_idx_type b_nc = b.cols ();
	  rcond = 0.;

	  // Calculate the 1-norm of matrix for rcond calculation
	  for (octave_idx_type j = 0; j < nr; j++)
	    {
	      double atmp = 0.;
	      for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
		atmp += std::abs(data(i));
	      if (atmp > anorm)
		anorm = atmp;
	    }

	  if (typ == SparseType::Permuted_Upper)
	    {
	      retval.resize (nr, b_nc);
	      octave_idx_type *perm = mattype.triangular_perm ();
	      OCTAVE_LOCAL_BUFFER (Complex, work, nr);

	      for (octave_idx_type j = 0; j < b_nc; j++)
		{
		  for (octave_idx_type i = 0; i < nr; i++)
		    work[i] = b(i,j);

		  for (octave_idx_type k = nr-1; k >= 0; k--)
		    {
		      octave_idx_type kidx = perm[k];

		      if (work[k] != 0.)
			{
			  if (ridx(cidx(kidx+1)-1) != k)
			    {
			      err = -2;
			      goto triangular_error;
			    }			    

			  Complex tmp = work[k] / data(cidx(kidx+1)-1);
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(kidx); 
			       i < cidx(kidx+1)-1; i++)
			    {
			      octave_idx_type iidx = ridx(i);
			      work[iidx] = work[iidx] - tmp * data(i);
			    }
			}
		    }

		  for (octave_idx_type i = 0; i < nr; i++)
		    retval (perm[i], j) = work[i];
		}

	      // Calculation of 1-norm of inv(*this)
	      for (octave_idx_type i = 0; i < nr; i++)
		work[i] = 0.;

	      for (octave_idx_type j = 0; j < nr; j++)
		{
		  work[j] = 1.;

		  for (octave_idx_type k = j; k >= 0; k--)
		    {
		      octave_idx_type iidx = perm[k];

		      if (work[k] != 0.)
			{
			  Complex tmp = work[k] / data(cidx(iidx+1)-1);
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(iidx); 
			       i < cidx(iidx+1)-1; i++)
			    {
			      octave_idx_type idx2 = ridx(i);
			      work[idx2] = work[idx2] - tmp * data(i);
			    }
			}
		    }
		  double atmp = 0;
		  for (octave_idx_type i = 0; i < j+1; i++)
		    {
		      atmp += std::abs(work[i]);
		      work[i] = 0.;
		    }
		  if (atmp > ainvnorm)
		    ainvnorm = atmp;
		}
	    }
	  else
	    {
	      retval = b;
	      Complex *x_vec = retval.fortran_vec ();

	      for (octave_idx_type j = 0; j < b_nc; j++)
		{
		  octave_idx_type offset = j * nr;
		  for (octave_idx_type k = nr-1; k >= 0; k--)
		    {
		      if (x_vec[k+offset] != 0.)
			{
			  if (ridx(cidx(k+1)-1) != k)
			    {
			      err = -2;
			      goto triangular_error;
			    }			    

			  Complex tmp = x_vec[k+offset] / 
			    data(cidx(k+1)-1);
			  x_vec[k+offset] = tmp;
			  for (octave_idx_type i = cidx(k); i < cidx(k+1)-1; i++)
			    {
			      octave_idx_type iidx = ridx(i);
			      x_vec[iidx+offset] = 
				x_vec[iidx+offset] - tmp * data(i);
			    }
			}
		    }
		}

	      // Calculation of 1-norm of inv(*this)
	      OCTAVE_LOCAL_BUFFER (Complex, work, nr);
	      for (octave_idx_type i = 0; i < nr; i++)
		work[i] = 0.;

	      for (octave_idx_type j = 0; j < nr; j++)
		{
		  work[j] = 1.;

		  for (octave_idx_type k = j; k >= 0; k--)
		    {
		      if (work[k] != 0.)
			{
			  Complex tmp = work[k] / data(cidx(k+1)-1);
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(k); i < cidx(k+1)-1; i++)
			    {
			      octave_idx_type iidx = ridx(i);
			      work[iidx] = work[iidx] - tmp * data(i);
			    }
			}
		    }
		  double atmp = 0;
		  for (octave_idx_type i = 0; i < j+1; i++)
		    {
		      atmp += std::abs(work[i]);
		      work[i] = 0.;
		    }
		  if (atmp > ainvnorm)
		    ainvnorm = atmp;
		}
	    }

	  rcond = 1. / ainvnorm / anorm;

	triangular_error:
	  if (err != 0)
	    {
	      if (sing_handler)
		sing_handler (rcond);
	      else
		(*current_liboctave_error_handler)
		  ("SparseComplexMatrix::solve matrix singular to machine precision, rcond = %g",
		   rcond);
	    }

	  volatile double rcond_plus_one = rcond + 1.0;

	  if (rcond_plus_one == 1.0 || xisnan (rcond))
	    {
	      err = -2;

	      if (sing_handler)
		sing_handler (rcond);
	      else
		(*current_liboctave_error_handler)
		  ("matrix singular to machine precision, rcond = %g",
		   rcond);
	    }
	}
      else
	(*current_liboctave_error_handler) ("incorrect matrix type");
    }

  return retval;
}

SparseComplexMatrix
SparseComplexMatrix::utsolve (SparseType &mattype, const SparseComplexMatrix& b,
		     octave_idx_type& err, double& rcond, 
		     solve_singularity_handler sing_handler) const
{
  SparseComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  err = 0;

  if (nr == 0 || nc == 0 || nr != nc || nr != b.rows ())
    (*current_liboctave_error_handler)
      ("matrix dimension mismatch solution of linear equations");
  else
    {
      // Print spparms("spumoni") info if requested
      int typ = mattype.type ();
      mattype.info ();
      
      if (typ == SparseType::Permuted_Upper ||
	  typ == SparseType::Upper)
	{
	  double anorm = 0.;
	  double ainvnorm = 0.;
	  rcond = 0.;

	  // Calculate the 1-norm of matrix for rcond calculation
	  for (octave_idx_type j = 0; j < nr; j++)
	    {
	      double atmp = 0.;
	      for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
		atmp += std::abs(data(i));
	      if (atmp > anorm)
		anorm = atmp;
	    }

	  octave_idx_type b_nr = b.rows ();
	  octave_idx_type b_nc = b.cols ();
	  octave_idx_type b_nz = b.nnz ();
	  retval = SparseComplexMatrix (b_nr, b_nc, b_nz);
	  retval.xcidx(0) = 0;
	  octave_idx_type ii = 0;
	  octave_idx_type x_nz = b_nz;

	  if (typ == SparseType::Permuted_Upper)
	    {
	      octave_idx_type *perm = mattype.triangular_perm ();
	      OCTAVE_LOCAL_BUFFER (Complex, work, nr);

	      OCTAVE_LOCAL_BUFFER (octave_idx_type, rperm, nr);
	      for (octave_idx_type i = 0; i < nr; i++)
		rperm[perm[i]] = i;

	      for (octave_idx_type j = 0; j < b_nc; j++)
		{
		  for (octave_idx_type i = 0; i < nr; i++)
		    work[i] = 0.;
		  for (octave_idx_type i = b.cidx(j); i < b.cidx(j+1); i++)
		    work[b.ridx(i)] = b.data(i);

		  for (octave_idx_type k = nr-1; k >= 0; k--)
		    {
		      octave_idx_type kidx = perm[k];

		      if (work[k] != 0.)
			{
			  if (ridx(cidx(kidx+1)-1) != k)
			    {
			      err = -2;
			      goto triangular_error;
			    }			    

			  Complex tmp = work[k] / data(cidx(kidx+1)-1);
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(kidx); 
			       i < cidx(kidx+1)-1; i++)
			    {
			      octave_idx_type iidx = ridx(i);
			      work[iidx] = work[iidx] - tmp * data(i);
			    }
			}
		    }

		  // Count non-zeros in work vector and adjust space in
		  // retval if needed
		  octave_idx_type new_nnz = 0;
		  for (octave_idx_type i = 0; i < nr; i++)
		    if (work[i] != 0.)
		      new_nnz++;

		  if (ii + new_nnz > x_nz)
		    {
		      // Resize the sparse matrix
		      octave_idx_type sz = new_nnz * (b_nc - j) + x_nz;
		      retval.change_capacity (sz);
		      x_nz = sz;
		    }

		  for (octave_idx_type i = 0; i < nr; i++)
		    if (work[rperm[i]] != 0.)
		      {
			retval.xridx(ii) = i;
			retval.xdata(ii++) = work[rperm[i]];
		      }
		  retval.xcidx(j+1) = ii;
		}

	      retval.maybe_compress ();

	      // Calculation of 1-norm of inv(*this)
	      for (octave_idx_type i = 0; i < nr; i++)
		work[i] = 0.;

	      for (octave_idx_type j = 0; j < nr; j++)
		{
		  work[j] = 1.;

		  for (octave_idx_type k = j; k >= 0; k--)
		    {
		      octave_idx_type iidx = perm[k];

		      if (work[k] != 0.)
			{
			  Complex tmp = work[k] / data(cidx(iidx+1)-1);
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(iidx); 
			       i < cidx(iidx+1)-1; i++)
			    {
			      octave_idx_type idx2 = ridx(i);
			      work[idx2] = work[idx2] - tmp * data(i);
			    }
			}
		    }
		  double atmp = 0;
		  for (octave_idx_type i = 0; i < j+1; i++)
		    {
		      atmp += std::abs(work[i]);
		      work[i] = 0.;
		    }
		  if (atmp > ainvnorm)
		    ainvnorm = atmp;
		}
	    }
	  else
	    {
	      OCTAVE_LOCAL_BUFFER (Complex, work, nr);

	      for (octave_idx_type j = 0; j < b_nc; j++)
		{
		  for (octave_idx_type i = 0; i < nr; i++)
		    work[i] = 0.;
		  for (octave_idx_type i = b.cidx(j); i < b.cidx(j+1); i++)
		    work[b.ridx(i)] = b.data(i);

		  for (octave_idx_type k = nr-1; k >= 0; k--)
		    {
		      if (work[k] != 0.)
			{
			  if (ridx(cidx(k+1)-1) != k)
			    {
			      err = -2;
			      goto triangular_error;
			    }			    

			  Complex tmp = work[k] / data(cidx(k+1)-1);
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(k); i < cidx(k+1)-1; i++)
			    {
			      octave_idx_type iidx = ridx(i);
			      work[iidx] = work[iidx] - tmp * data(i);
			    }
			}
		    }

		  // Count non-zeros in work vector and adjust space in
		  // retval if needed
		  octave_idx_type new_nnz = 0;
		  for (octave_idx_type i = 0; i < nr; i++)
		    if (work[i] != 0.)
		      new_nnz++;

		  if (ii + new_nnz > x_nz)
		    {
		      // Resize the sparse matrix
		      octave_idx_type sz = new_nnz * (b_nc - j) + x_nz;
		      retval.change_capacity (sz);
		      x_nz = sz;
		    }

		  for (octave_idx_type i = 0; i < nr; i++)
		    if (work[i] != 0.)
		      {
			retval.xridx(ii) = i;
			retval.xdata(ii++) = work[i];
		      }
		  retval.xcidx(j+1) = ii;
		}

	      retval.maybe_compress ();

	      // Calculation of 1-norm of inv(*this)
	      for (octave_idx_type i = 0; i < nr; i++)
		work[i] = 0.;

	      for (octave_idx_type j = 0; j < nr; j++)
		{
		  work[j] = 1.;

		  for (octave_idx_type k = j; k >= 0; k--)
		    {
		      if (work[k] != 0.)
			{
			  Complex tmp = work[k] / data(cidx(k+1)-1);
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(k); i < cidx(k+1)-1; i++)
			    {
			      octave_idx_type iidx = ridx(i);
			      work[iidx] = work[iidx] - tmp * data(i);
			    }
			}
		    }
		  double atmp = 0;
		  for (octave_idx_type i = 0; i < j+1; i++)
		    {
		      atmp += std::abs(work[i]);
		      work[i] = 0.;
		    }
		  if (atmp > ainvnorm)
		    ainvnorm = atmp;
		}
	    }

	  rcond = 1. / ainvnorm / anorm;

	triangular_error:
	  if (err != 0)
	    {
	      if (sing_handler)
		sing_handler (rcond);
	      else
		(*current_liboctave_error_handler)
		  ("SparseComplexMatrix::solve matrix singular to machine precision, rcond = %g",
		   rcond);
	    }

	  volatile double rcond_plus_one = rcond + 1.0;

	  if (rcond_plus_one == 1.0 || xisnan (rcond))
	    {
	      err = -2;

	      if (sing_handler)
		sing_handler (rcond);
	      else
		(*current_liboctave_error_handler)
		  ("matrix singular to machine precision, rcond = %g",
		   rcond);
	    }
	}
      else
	(*current_liboctave_error_handler) ("incorrect matrix type");
    }

  return retval;
}

ComplexMatrix
SparseComplexMatrix::ltsolve (SparseType &mattype, const Matrix& b, octave_idx_type& err, 
		   double& rcond, solve_singularity_handler sing_handler) const
{
  ComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  err = 0;

  if (nr == 0 || nc == 0 || nr != nc || nr != b.rows ())
    (*current_liboctave_error_handler)
      ("matrix dimension mismatch solution of linear equations");
  else
    {
      // Print spparms("spumoni") info if requested
      int typ = mattype.type ();
      mattype.info ();
      
      if (typ == SparseType::Permuted_Lower ||
	  typ == SparseType::Lower)
	{
	  double anorm = 0.;
	  double ainvnorm = 0.;
	  octave_idx_type b_cols = b.cols ();
	  rcond = 0.;

	  // Calculate the 1-norm of matrix for rcond calculation
	  for (octave_idx_type j = 0; j < nr; j++)
	    {
	      double atmp = 0.;
	      for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
		atmp += std::abs(data(i));
	      if (atmp > anorm)
		anorm = atmp;
	    }

	  if (typ == SparseType::Permuted_Lower)
	    {
	      retval.resize (b.rows (), b.cols ());
	      OCTAVE_LOCAL_BUFFER (Complex, work, nr);
	      octave_idx_type *perm = mattype.triangular_perm ();

	      for (octave_idx_type j = 0; j < b_cols; j++)
		{
		  for (octave_idx_type i = 0; i < nr; i++)
		    work[perm[i]] = b(i,j);

		  for (octave_idx_type k = 0; k < nr; k++)
		    {
		      if (work[k] != 0.)
			{
			  octave_idx_type minr = nr;
			  octave_idx_type mini = 0;

			  for (octave_idx_type i = cidx(k); i < cidx(k+1); i++)
			    if (perm[ridx(i)] < minr)
			      {
				minr = perm[ridx(i)];
				mini = i;
			      }

			  if (minr != k)
			    {
			      err = -2;
			      goto triangular_error;
			    }			    

			  Complex tmp = work[k] / data(mini);
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(k); i < cidx(k+1); i++)
			    {
			      if (i == mini)
				continue;

			      octave_idx_type iidx = perm[ridx(i)];
			      work[iidx] = work[iidx] - tmp * data(i);
			    }
			}
		    }

		  for (octave_idx_type i = 0; i < nr; i++)
		    retval (i, j) = work[i];
		}

	      // Calculation of 1-norm of inv(*this)
	      for (octave_idx_type i = 0; i < nr; i++)
		work[i] = 0.;

	      for (octave_idx_type j = 0; j < nr; j++)
		{
		  work[j] = 1.;

		  for (octave_idx_type k = 0; k < nr; k++)
		    {
		      if (work[k] != 0.)
			{
			  octave_idx_type minr = nr;
			  octave_idx_type mini = 0;

			  for (octave_idx_type i = cidx(k); i < cidx(k+1); i++)
			    if (perm[ridx(i)] < minr)
			      {
				minr = perm[ridx(i)];
				mini = i;
			      }

			  Complex tmp = work[k] / data(mini);
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(k); i < cidx(k+1); i++)
			    {
			      if (i == mini)
				continue;

			      octave_idx_type iidx = perm[ridx(i)];
			      work[iidx] = work[iidx] - tmp * data(i);
			    }
			}
		    }

		  double atmp = 0;
		  for (octave_idx_type i = j; i < nr; i++)
		    {
		      atmp += std::abs(work[i]);
		      work[i] = 0.;
		    }
		  if (atmp > ainvnorm)
		    ainvnorm = atmp;
		}
	    }
	  else
	    {
	      retval = ComplexMatrix (b);
	      Complex *x_vec = retval.fortran_vec ();

	      for (octave_idx_type j = 0; j < b_cols; j++)
		{
		  octave_idx_type offset = j * nr;
		  for (octave_idx_type k = 0; k < nr; k++)
		    {
		      if (x_vec[k+offset] != 0.)
			{
			  if (ridx(cidx(k)) != k)
			    {
			      err = -2;
			      goto triangular_error;
			    }			    

			  Complex tmp = x_vec[k+offset] / 
			    data(cidx(k));
			  x_vec[k+offset] = tmp;
			  for (octave_idx_type i = cidx(k)+1; i < cidx(k+1); i++)
			    {
			      octave_idx_type iidx = ridx(i);
			      x_vec[iidx+offset] = 
				x_vec[iidx+offset] - tmp * data(i);
			    }
			}
		    }
		}

	      // Calculation of 1-norm of inv(*this)
	      OCTAVE_LOCAL_BUFFER (Complex, work, nr);
	      for (octave_idx_type i = 0; i < nr; i++)
		work[i] = 0.;

	      for (octave_idx_type j = 0; j < nr; j++)
		{
		  work[j] = 1.;

		  for (octave_idx_type k = j; k < nr; k++)
		    {

		      if (work[k] != 0.)
			{
			  Complex tmp = work[k] / data(cidx(k));
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(k)+1; i < cidx(k+1); i++)
			    {
			      octave_idx_type iidx = ridx(i);
			      work[iidx] = work[iidx] - tmp * data(i);
			    }
			}
		    }
		  double atmp = 0;
		  for (octave_idx_type i = j; i < nr; i++)
		    {
		      atmp += std::abs(work[i]);
		      work[i] = 0.;
		    }
		  if (atmp > ainvnorm)
		    ainvnorm = atmp;
		}
	    }

	  rcond = 1. / ainvnorm / anorm;

	triangular_error:
	  if (err != 0)
	    {
	      if (sing_handler)
		sing_handler (rcond);
	      else
		(*current_liboctave_error_handler)
		  ("SparseComplexMatrix::solve matrix singular to machine precision, rcond = %g",
		   rcond);
	    }

	  volatile double rcond_plus_one = rcond + 1.0;

	  if (rcond_plus_one == 1.0 || xisnan (rcond))
	    {
	      err = -2;

	      if (sing_handler)
		sing_handler (rcond);
	      else
		(*current_liboctave_error_handler)
		  ("matrix singular to machine precision, rcond = %g",
		   rcond);
	    }
	}
      else
	(*current_liboctave_error_handler) ("incorrect matrix type");
    }

  return retval;
}

SparseComplexMatrix
SparseComplexMatrix::ltsolve (SparseType &mattype, const SparseMatrix& b, 
			octave_idx_type& err, double& rcond, 
			solve_singularity_handler sing_handler) const
{
  SparseComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  err = 0;

  if (nr == 0 || nc == 0 || nr != nc || nr != b.rows ())
    (*current_liboctave_error_handler)
      ("matrix dimension mismatch solution of linear equations");
  else
    {
      // Print spparms("spumoni") info if requested
      int typ = mattype.type ();
      mattype.info ();
      
      if (typ == SparseType::Permuted_Lower ||
	  typ == SparseType::Lower)
	{
	  double anorm = 0.;
	  double ainvnorm = 0.;
	  rcond = 0.;

	  // Calculate the 1-norm of matrix for rcond calculation
	  for (octave_idx_type j = 0; j < nr; j++)
	    {
	      double atmp = 0.;
	      for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
		atmp += std::abs(data(i));
	      if (atmp > anorm)
		anorm = atmp;
	    }

	  octave_idx_type b_nr = b.rows ();
	  octave_idx_type b_nc = b.cols ();
	  octave_idx_type b_nz = b.nnz ();
	  retval = SparseComplexMatrix (b_nr, b_nc, b_nz);
	  retval.xcidx(0) = 0;
	  octave_idx_type ii = 0;
	  octave_idx_type x_nz = b_nz;

	  if (typ == SparseType::Permuted_Lower)
	    {
	      OCTAVE_LOCAL_BUFFER (Complex, work, nr);
	      octave_idx_type *perm = mattype.triangular_perm ();

	      for (octave_idx_type j = 0; j < b_nc; j++)
		{
		  for (octave_idx_type i = 0; i < nr; i++)
		    work[i] = 0.;
		  for (octave_idx_type i = b.cidx(j); i < b.cidx(j+1); i++)
		    work[perm[b.ridx(i)]] = b.data(i);

		  for (octave_idx_type k = 0; k < nr; k++)
		    {
		      if (work[k] != 0.)
			{
			  octave_idx_type minr = nr;
			  octave_idx_type mini = 0;

			  for (octave_idx_type i = cidx(k); i < cidx(k+1); i++)
			    if (perm[ridx(i)] < minr)
			      {
				minr = perm[ridx(i)];
				mini = i;
			      }

			  if (minr != k)
			    {
			      err = -2;
			      goto triangular_error;
			    }			    

			  Complex tmp = work[k] / data(mini);
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(k); i < cidx(k+1); i++)
			    {
			      if (i == mini)
				continue;

			      octave_idx_type iidx = perm[ridx(i)];
			      work[iidx] = work[iidx] - tmp * data(i);
			    }
			}
		    }

		  // Count non-zeros in work vector and adjust space in
		  // retval if needed
		  octave_idx_type new_nnz = 0;
		  for (octave_idx_type i = 0; i < nr; i++)
		    if (work[i] != 0.)
		      new_nnz++;

		  if (ii + new_nnz > x_nz)
		    {
		      // Resize the sparse matrix
		      octave_idx_type sz = new_nnz * (b_nc - j) + x_nz;
		      retval.change_capacity (sz);
		      x_nz = sz;
		    }

		  for (octave_idx_type i = 0; i < nr; i++)
		    if (work[i] != 0.)
		      {
			retval.xridx(ii) = i;
			retval.xdata(ii++) = work[i];
		      }
		  retval.xcidx(j+1) = ii;
		}

	      retval.maybe_compress ();

	      // Calculation of 1-norm of inv(*this)
	      for (octave_idx_type i = 0; i < nr; i++)
		work[i] = 0.;

	      for (octave_idx_type j = 0; j < nr; j++)
		{
		  work[j] = 1.;

		  for (octave_idx_type k = 0; k < nr; k++)
		    {
		      if (work[k] != 0.)
			{
			  octave_idx_type minr = nr;
			  octave_idx_type mini = 0;

			  for (octave_idx_type i = cidx(k); i < cidx(k+1); i++)
			    if (perm[ridx(i)] < minr)
			      {
				minr = perm[ridx(i)];
				mini = i;
			      }

			  Complex tmp = work[k] / data(mini);
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(k); i < cidx(k+1); i++)
			    {
			      if (i == mini)
				continue;

			      octave_idx_type iidx = perm[ridx(i)];
			      work[iidx] = work[iidx] - tmp * data(i);
			    }
			}
		    }

		  double atmp = 0;
		  for (octave_idx_type i = j; i < nr; i++)
		    {
		      atmp += std::abs(work[i]);
		      work[i] = 0.;
		    }
		  if (atmp > ainvnorm)
		    ainvnorm = atmp;
		}
	    }
	  else
	    {
	      OCTAVE_LOCAL_BUFFER (Complex, work, nr);

	      for (octave_idx_type j = 0; j < b_nc; j++)
		{
		  for (octave_idx_type i = 0; i < nr; i++)
		    work[i] = 0.;
		  for (octave_idx_type i = b.cidx(j); i < b.cidx(j+1); i++)
		    work[b.ridx(i)] = b.data(i);

		  for (octave_idx_type k = 0; k < nr; k++)
		    {
		      if (work[k] != 0.)
			{
			  if (ridx(cidx(k)) != k)
			    {
			      err = -2;
			      goto triangular_error;
			    }			    

			  Complex tmp = work[k] / data(cidx(k));
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(k)+1; i < cidx(k+1); i++)
			    {
			      octave_idx_type iidx = ridx(i);
			      work[iidx] = work[iidx] - tmp * data(i);
			    }
			}
		    }

		  // Count non-zeros in work vector and adjust space in
		  // retval if needed
		  octave_idx_type new_nnz = 0;
		  for (octave_idx_type i = 0; i < nr; i++)
		    if (work[i] != 0.)
		      new_nnz++;

		  if (ii + new_nnz > x_nz)
		    {
		      // Resize the sparse matrix
		      octave_idx_type sz = new_nnz * (b_nc - j) + x_nz;
		      retval.change_capacity (sz);
		      x_nz = sz;
		    }

		  for (octave_idx_type i = 0; i < nr; i++)
		    if (work[i] != 0.)
		      {
			retval.xridx(ii) = i;
			retval.xdata(ii++) = work[i];
		      }
		  retval.xcidx(j+1) = ii;
		}

	      retval.maybe_compress ();

	      // Calculation of 1-norm of inv(*this)
	      for (octave_idx_type i = 0; i < nr; i++)
		work[i] = 0.;

	      for (octave_idx_type j = 0; j < nr; j++)
		{
		  work[j] = 1.;

		  for (octave_idx_type k = j; k < nr; k++)
		    {

		      if (work[k] != 0.)
			{
			  Complex tmp = work[k] / data(cidx(k));
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(k)+1; i < cidx(k+1); i++)
			    {
			      octave_idx_type iidx = ridx(i);
			      work[iidx] = work[iidx] - tmp * data(i);
			    }
			}
		    }
		  double atmp = 0;
		  for (octave_idx_type i = j; i < nr; i++)
		    {
		      atmp += std::abs(work[i]);
		      work[i] = 0.;
		    }
		  if (atmp > ainvnorm)
		    ainvnorm = atmp;
		}

	    }

	  rcond = 1. / ainvnorm / anorm;

	triangular_error:
	  if (err != 0)
	    {
	      if (sing_handler)
		sing_handler (rcond);
	      else
		(*current_liboctave_error_handler)
		  ("SparseComplexMatrix::solve matrix singular to machine precision, rcond = %g",
		   rcond);
	    }

	  volatile double rcond_plus_one = rcond + 1.0;

	  if (rcond_plus_one == 1.0 || xisnan (rcond))
	    {
	      err = -2;

	      if (sing_handler)
		sing_handler (rcond);
	      else
		(*current_liboctave_error_handler)
		  ("matrix singular to machine precision, rcond = %g",
		   rcond);
	    }
	}
      else
	(*current_liboctave_error_handler) ("incorrect matrix type");
    }

  return retval;
}

ComplexMatrix
SparseComplexMatrix::ltsolve (SparseType &mattype, const ComplexMatrix& b,
			octave_idx_type& err, double& rcond,
			solve_singularity_handler sing_handler) const
{
  ComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  err = 0;

  if (nr == 0 || nc == 0 || nr != nc || nr != b.rows ())
    (*current_liboctave_error_handler)
      ("matrix dimension mismatch solution of linear equations");
  else
    {
      // Print spparms("spumoni") info if requested
      int typ = mattype.type ();
      mattype.info ();
      
      if (typ == SparseType::Permuted_Lower ||
	  typ == SparseType::Lower)
	{
	  double anorm = 0.;
	  double ainvnorm = 0.;
	  octave_idx_type b_nc = b.cols ();
	  rcond = 0.;

	  // Calculate the 1-norm of matrix for rcond calculation
	  for (octave_idx_type j = 0; j < nr; j++)
	    {
	      double atmp = 0.;
	      for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
		atmp += std::abs(data(i));
	      if (atmp > anorm)
		anorm = atmp;
	    }

	  if (typ == SparseType::Permuted_Lower)
	    {
	      retval.resize (b.rows (), b.cols ());
	      OCTAVE_LOCAL_BUFFER (Complex, work, nr);
	      octave_idx_type *perm = mattype.triangular_perm ();

	      for (octave_idx_type j = 0; j < b_nc; j++)
		{
		  for (octave_idx_type i = 0; i < nr; i++)
		    work[perm[i]] = b(i,j);

		  for (octave_idx_type k = 0; k < nr; k++)
		    {
		      if (work[k] != 0.)
			{
			  octave_idx_type minr = nr;
			  octave_idx_type mini = 0;

			  for (octave_idx_type i = cidx(k); i < cidx(k+1); i++)
			    if (perm[ridx(i)] < minr)
			      {
				minr = perm[ridx(i)];
				mini = i;
			      }

			  if (minr != k)
			    {
			      err = -2;
			      goto triangular_error;
			    }			    

			  Complex tmp = work[k] / data(mini);
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(k); i < cidx(k+1); i++)
			    {
			      if (i == mini)
				continue;

			      octave_idx_type iidx = perm[ridx(i)];
			      work[iidx] = work[iidx] - tmp * data(i);
			    }
			}
		    }

		  for (octave_idx_type i = 0; i < nr; i++)
		    retval (i, j) = work[i];
		}

	      // Calculation of 1-norm of inv(*this)
	      for (octave_idx_type i = 0; i < nr; i++)
		work[i] = 0.;

	      for (octave_idx_type j = 0; j < nr; j++)
		{
		  work[j] = 1.;

		  for (octave_idx_type k = 0; k < nr; k++)
		    {
		      if (work[k] != 0.)
			{
			  octave_idx_type minr = nr;
			  octave_idx_type mini = 0;

			  for (octave_idx_type i = cidx(k); i < cidx(k+1); i++)
			    if (perm[ridx(i)] < minr)
			      {
				minr = perm[ridx(i)];
				mini = i;
			      }

			  Complex tmp = work[k] / data(mini);
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(k); i < cidx(k+1); i++)
			    {
			      if (i == mini)
				continue;

			      octave_idx_type iidx = perm[ridx(i)];
			      work[iidx] = work[iidx] - tmp * data(i);
			    }
			}
		    }

		  double atmp = 0;
		  for (octave_idx_type i = j; i < nr; i++)
		    {
		      atmp += std::abs(work[i]);
		      work[i] = 0.;
		    }
		  if (atmp > ainvnorm)
		    ainvnorm = atmp;
		}
	    }
	  else
	    {
	      retval = b;
	      Complex *x_vec = retval.fortran_vec ();

	      for (octave_idx_type j = 0; j < b_nc; j++)
		{
		  octave_idx_type offset = j * nr;
		  for (octave_idx_type k = 0; k < nr; k++)
		    {
		      if (x_vec[k+offset] != 0.)
			{
			  if (ridx(cidx(k)) != k)
			    {
			      err = -2;
			      goto triangular_error;
			    }			    

			  Complex tmp = x_vec[k+offset] / 
			    data(cidx(k));
			  x_vec[k+offset] = tmp;
			  for (octave_idx_type i = cidx(k)+1; i < cidx(k+1); i++)
			    {
			      octave_idx_type iidx = ridx(i);
			      x_vec[iidx+offset] = 
				x_vec[iidx+offset] - tmp * data(i);
			    }
			}
		    }
		}

	      // Calculation of 1-norm of inv(*this)
	      OCTAVE_LOCAL_BUFFER (Complex, work, nr);
	      for (octave_idx_type i = 0; i < nr; i++)
		work[i] = 0.;

	      for (octave_idx_type j = 0; j < nr; j++)
		{
		  work[j] = 1.;

		  for (octave_idx_type k = j; k < nr; k++)
		    {

		      if (work[k] != 0.)
			{
			  Complex tmp = work[k] / data(cidx(k));
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(k)+1; i < cidx(k+1); i++)
			    {
			      octave_idx_type iidx = ridx(i);
			      work[iidx] = work[iidx] - tmp * data(i);
			    }
			}
		    }
		  double atmp = 0;
		  for (octave_idx_type i = j; i < nr; i++)
		    {
		      atmp += std::abs(work[i]);
		      work[i] = 0.;
		    }
		  if (atmp > ainvnorm)
		    ainvnorm = atmp;
		}

	    }

	  rcond = 1. / ainvnorm / anorm;

	triangular_error:
	  if (err != 0)
	    {
	      if (sing_handler)
		sing_handler (rcond);
	      else
		(*current_liboctave_error_handler)
		  ("SparseComplexMatrix::solve matrix singular to machine precision, rcond = %g",
		   rcond);
	    }

	  volatile double rcond_plus_one = rcond + 1.0;

	  if (rcond_plus_one == 1.0 || xisnan (rcond))
	    {
	      err = -2;

	      if (sing_handler)
		sing_handler (rcond);
	      else
		(*current_liboctave_error_handler)
		  ("matrix singular to machine precision, rcond = %g",
		   rcond);
	    }
	}
      else
	(*current_liboctave_error_handler) ("incorrect matrix type");
    }

  return retval;
}

SparseComplexMatrix
SparseComplexMatrix::ltsolve (SparseType &mattype, const SparseComplexMatrix& b,
		     octave_idx_type& err, double& rcond, 
		     solve_singularity_handler sing_handler) const
{
  SparseComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  err = 0;

  if (nr == 0 || nc == 0 || nr != nc || nr != b.rows ())
    (*current_liboctave_error_handler)
      ("matrix dimension mismatch solution of linear equations");
  else
    {
      // Print spparms("spumoni") info if requested
      int typ = mattype.type ();
      mattype.info ();
      
      if (typ == SparseType::Permuted_Lower ||
	  typ == SparseType::Lower)
	{
	  double anorm = 0.;
	  double ainvnorm = 0.;
	  rcond = 0.;

	  // Calculate the 1-norm of matrix for rcond calculation
	  for (octave_idx_type j = 0; j < nr; j++)
	    {
	      double atmp = 0.;
	      for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
		atmp += std::abs(data(i));
	      if (atmp > anorm)
		anorm = atmp;
	    }

	  octave_idx_type b_nr = b.rows ();
	  octave_idx_type b_nc = b.cols ();
	  octave_idx_type b_nz = b.nnz ();
	  retval = SparseComplexMatrix (b_nr, b_nc, b_nz);
	  retval.xcidx(0) = 0;
	  octave_idx_type ii = 0;
	  octave_idx_type x_nz = b_nz;

	  if (typ == SparseType::Permuted_Lower)
	    {
	      OCTAVE_LOCAL_BUFFER (Complex, work, nr);
	      octave_idx_type *perm = mattype.triangular_perm ();

	      for (octave_idx_type j = 0; j < b_nc; j++)
		{
		  for (octave_idx_type i = 0; i < nr; i++)
		    work[i] = 0.;
		  for (octave_idx_type i = b.cidx(j); i < b.cidx(j+1); i++)
		    work[perm[b.ridx(i)]] = b.data(i);

		  for (octave_idx_type k = 0; k < nr; k++)
		    {
		      if (work[k] != 0.)
			{
			  octave_idx_type minr = nr;
			  octave_idx_type mini = 0;

			  for (octave_idx_type i = cidx(k); i < cidx(k+1); i++)
			    if (perm[ridx(i)] < minr)
			      {
				minr = perm[ridx(i)];
				mini = i;
			      }

			  if (minr != k)
			    {
			      err = -2;
			      goto triangular_error;
			    }			    

			  Complex tmp = work[k] / data(mini);
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(k); i < cidx(k+1); i++)
			    {
			      if (i == mini)
				continue;

			      octave_idx_type iidx = perm[ridx(i)];
			      work[iidx] = work[iidx] - tmp * data(i);
			    }
			}
		    }

		  // Count non-zeros in work vector and adjust space in
		  // retval if needed
		  octave_idx_type new_nnz = 0;
		  for (octave_idx_type i = 0; i < nr; i++)
		    if (work[i] != 0.)
		      new_nnz++;

		  if (ii + new_nnz > x_nz)
		    {
		      // Resize the sparse matrix
		      octave_idx_type sz = new_nnz * (b_nc - j) + x_nz;
		      retval.change_capacity (sz);
		      x_nz = sz;
		    }

		  for (octave_idx_type i = 0; i < nr; i++)
		    if (work[i] != 0.)
		      {
			retval.xridx(ii) = i;
			retval.xdata(ii++) = work[i];
		      }
		  retval.xcidx(j+1) = ii;
		}

	      retval.maybe_compress ();

	      // Calculation of 1-norm of inv(*this)
	      for (octave_idx_type i = 0; i < nr; i++)
		work[i] = 0.;

	      for (octave_idx_type j = 0; j < nr; j++)
		{
		  work[j] = 1.;

		  for (octave_idx_type k = 0; k < nr; k++)
		    {
		      if (work[k] != 0.)
			{
			  octave_idx_type minr = nr;
			  octave_idx_type mini = 0;

			  for (octave_idx_type i = cidx(k); i < cidx(k+1); i++)
			    if (perm[ridx(i)] < minr)
			      {
				minr = perm[ridx(i)];
				mini = i;
			      }

			  Complex tmp = work[k] / data(mini);
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(k); i < cidx(k+1); i++)
			    {
			      if (i == mini)
				continue;

			      octave_idx_type iidx = perm[ridx(i)];
			      work[iidx] = work[iidx] - tmp * data(i);
			    }
			}
		    }

		  double atmp = 0;
		  for (octave_idx_type i = j; i < nr; i++)
		    {
		      atmp += std::abs(work[i]);
		      work[i] = 0.;
		    }
		  if (atmp > ainvnorm)
		    ainvnorm = atmp;
		}
	    }
	  else
	    {
	      OCTAVE_LOCAL_BUFFER (Complex, work, nr);

	      for (octave_idx_type j = 0; j < b_nc; j++)
		{
		  for (octave_idx_type i = 0; i < nr; i++)
		    work[i] = 0.;
		  for (octave_idx_type i = b.cidx(j); i < b.cidx(j+1); i++)
		    work[b.ridx(i)] = b.data(i);

		  for (octave_idx_type k = 0; k < nr; k++)
		    {
		      if (work[k] != 0.)
			{
			  if (ridx(cidx(k)) != k)
			    {
			      err = -2;
			      goto triangular_error;
			    }			    

			  Complex tmp = work[k] / data(cidx(k));
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(k)+1; i < cidx(k+1); i++)
			    {
			      octave_idx_type iidx = ridx(i);
			      work[iidx] = work[iidx] - tmp * data(i);
			    }
			}
		    }

		  // Count non-zeros in work vector and adjust space in
		  // retval if needed
		  octave_idx_type new_nnz = 0;
		  for (octave_idx_type i = 0; i < nr; i++)
		    if (work[i] != 0.)
		      new_nnz++;

		  if (ii + new_nnz > x_nz)
		    {
		      // Resize the sparse matrix
		      octave_idx_type sz = new_nnz * (b_nc - j) + x_nz;
		      retval.change_capacity (sz);
		      x_nz = sz;
		    }

		  for (octave_idx_type i = 0; i < nr; i++)
		    if (work[i] != 0.)
		      {
			retval.xridx(ii) = i;
			retval.xdata(ii++) = work[i];
		      }
		  retval.xcidx(j+1) = ii;
		}

	      retval.maybe_compress ();

	      // Calculation of 1-norm of inv(*this)
	      for (octave_idx_type i = 0; i < nr; i++)
		work[i] = 0.;

	      for (octave_idx_type j = 0; j < nr; j++)
		{
		  work[j] = 1.;

		  for (octave_idx_type k = j; k < nr; k++)
		    {

		      if (work[k] != 0.)
			{
			  Complex tmp = work[k] / data(cidx(k));
			  work[k] = tmp;
			  for (octave_idx_type i = cidx(k)+1; i < cidx(k+1); i++)
			    {
			      octave_idx_type iidx = ridx(i);
			      work[iidx] = work[iidx] - tmp * data(i);
			    }
			}
		    }
		  double atmp = 0;
		  for (octave_idx_type i = j; i < nr; i++)
		    {
		      atmp += std::abs(work[i]);
		      work[i] = 0.;
		    }
		  if (atmp > ainvnorm)
		    ainvnorm = atmp;
		}

	    }

	  rcond = 1. / ainvnorm / anorm;

	triangular_error:
	  if (err != 0)
	    {
	      if (sing_handler)
		sing_handler (rcond);
	      else
		(*current_liboctave_error_handler)
		  ("SparseComplexMatrix::solve matrix singular to machine precision, rcond = %g",
		   rcond);
	    }

	  volatile double rcond_plus_one = rcond + 1.0;

	  if (rcond_plus_one == 1.0 || xisnan (rcond))
	    {
	      err = -2;

	      if (sing_handler)
		sing_handler (rcond);
	      else
		(*current_liboctave_error_handler)
		  ("matrix singular to machine precision, rcond = %g",
		   rcond);
	    }
	}
      else
	(*current_liboctave_error_handler) ("incorrect matrix type");
    }

  return retval;
}

ComplexMatrix
SparseComplexMatrix::trisolve (SparseType &mattype, const Matrix& b, octave_idx_type& err, 
			       double& rcond,
			       solve_singularity_handler sing_handler) const
{
  ComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  err = 0;

  if (nr == 0 || nc == 0 || nr != nc || nr != b.rows ())
    (*current_liboctave_error_handler)
      ("matrix dimension mismatch solution of linear equations");
  else
    {
      // Print spparms("spumoni") info if requested
      volatile int typ = mattype.type ();
      mattype.info ();
      
      if (typ == SparseType::Tridiagonal_Hermitian)
	{
	  OCTAVE_LOCAL_BUFFER (double, D, nr);
	  OCTAVE_LOCAL_BUFFER (Complex, DL, nr - 1);

	  if (mattype.is_dense ())
	    {
	      octave_idx_type ii = 0;

	      for (octave_idx_type j = 0; j < nc-1; j++)
		{
		  D[j] = std::real(data(ii++));
		  DL[j] = data(ii);
		  ii += 2;
		}
	      D[nc-1] = std::real(data(ii));
	    }
	  else
	    {
	      D[0] = 0.;
	      for (octave_idx_type i = 0; i < nr - 1; i++)
		{
		  D[i+1] = 0.;
		  DL[i] = 0.;
		}

	      for (octave_idx_type j = 0; j < nc; j++)
		for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
		  {
		    if (ridx(i) == j)
		      D[j] = std::real(data(i));
		    else if (ridx(i) == j + 1)
		      DL[j] = data(i);
		  }
	    }
	      
	  octave_idx_type b_nc = b.cols();
	  retval = ComplexMatrix (b);
	  Complex *result = retval.fortran_vec ();

	  F77_XFCN (zptsv, ZPTSV, (nr, b_nc, D, DL, result, 
				   b.rows(), err));

	  if (f77_exception_encountered)
	    (*current_liboctave_error_handler) 
	      ("unrecoverable error in zptsv");
	  else if (err != 0)
	    {
	      err = 0;
	      mattype.mark_as_unsymmetric ();
	      typ = SparseType::Tridiagonal;
	    }
	  else 
	    rcond = 1.;
	}

      if (typ == SparseType::Tridiagonal)
	{
	  OCTAVE_LOCAL_BUFFER (Complex, DU, nr - 1);
	  OCTAVE_LOCAL_BUFFER (Complex, D, nr);
	  OCTAVE_LOCAL_BUFFER (Complex, DL, nr - 1);

	  if (mattype.is_dense ())
	    {
	      octave_idx_type ii = 0;

	      for (octave_idx_type j = 0; j < nc-1; j++)
		{
		  D[j] = data(ii++);
		  DL[j] = data(ii++);
		  DU[j] = data(ii++);
		}
	      D[nc-1] = data(ii);
	    }
	  else
	    {
	      D[0] = 0.;
	      for (octave_idx_type i = 0; i < nr - 1; i++)
		{
		  D[i+1] = 0.;
		  DL[i] = 0.;
		  DU[i] = 0.;
		}

	      for (octave_idx_type j = 0; j < nc; j++)
		for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
		  {
		    if (ridx(i) == j)
		      D[j] = data(i);
		    else if (ridx(i) == j + 1)
		      DL[j] = data(i);
		    else if (ridx(i) == j - 1)
		      DU[j-1] = data(i);
		  }
	    }

	  octave_idx_type b_nc = b.cols();
	  retval = ComplexMatrix (b);
	  Complex *result = retval.fortran_vec ();

	  F77_XFCN (zgtsv, ZGTSV, (nr, b_nc, DL, D, DU, result, 
				   b.rows(), err));

	  if (f77_exception_encountered)
	    (*current_liboctave_error_handler) 
	      ("unrecoverable error in zgtsv");
	  else if (err != 0)
	    {
	      rcond = 0.;
	      err = -2;

	      if (sing_handler)
		sing_handler (rcond);
	      else
		(*current_liboctave_error_handler)
		  ("matrix singular to machine precision");

	    } 
	  else 
	    rcond = 1.;
	}
      else if (typ != SparseType::Tridiagonal_Hermitian)
	       (*current_liboctave_error_handler) ("incorrect matrix type");
    }

  return retval;
}

SparseComplexMatrix
SparseComplexMatrix::trisolve (SparseType &mattype, const SparseMatrix& b,
		     octave_idx_type& err, double& rcond, 
		     solve_singularity_handler sing_handler) const
{
  SparseComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  err = 0;

  if (nr == 0 || nc == 0 || nr != nc || nr != b.rows ())
    (*current_liboctave_error_handler)
      ("matrix dimension mismatch solution of linear equations");
  else
    {
      // Print spparms("spumoni") info if requested
      int typ = mattype.type ();
      mattype.info ();
      
      // Note can't treat symmetric case as there is no dpttrf function
      if (typ == SparseType::Tridiagonal ||
	  typ == SparseType::Tridiagonal_Hermitian)
	{
	  OCTAVE_LOCAL_BUFFER (Complex, DU2, nr - 2);
	  OCTAVE_LOCAL_BUFFER (Complex, DU, nr - 1);
	  OCTAVE_LOCAL_BUFFER (Complex, D, nr);
	  OCTAVE_LOCAL_BUFFER (Complex, DL, nr - 1);
	  Array<octave_idx_type> ipvt (nr);
	  octave_idx_type *pipvt = ipvt.fortran_vec ();

	  if (mattype.is_dense ())
	    {
	      octave_idx_type ii = 0;

	      for (octave_idx_type j = 0; j < nc-1; j++)
		{
		  D[j] = data(ii++);
		  DL[j] = data(ii++);
		  DU[j] = data(ii++);
		}
	      D[nc-1] = data(ii);
	    }
	  else
	    {
	      D[0] = 0.;
	      for (octave_idx_type i = 0; i < nr - 1; i++)
		{
		  D[i+1] = 0.;
		  DL[i] = 0.;
		  DU[i] = 0.;
		}

	      for (octave_idx_type j = 0; j < nc; j++)
		for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
		  {
		    if (ridx(i) == j)
		      D[j] = data(i);
		    else if (ridx(i) == j + 1)
		      DL[j] = data(i);
		    else if (ridx(i) == j - 1)
		      DU[j-1] = data(i);
		  }
	    }

	  F77_XFCN (zgttrf, ZGTTRF, (nr, DL, D, DU, DU2, pipvt, err));

	  if (f77_exception_encountered)
	    (*current_liboctave_error_handler) 
	      ("unrecoverable error in zgttrf");
	  else
	    {
	      rcond = 0.0;
	      if (err != 0) 
		{
		  err = -2;

		  if (sing_handler)
		    sing_handler (rcond);
		  else
		    (*current_liboctave_error_handler)
		      ("matrix singular to machine precision");

		} 
	      else 
		{
		  char job = 'N';
		  volatile octave_idx_type x_nz = b.nnz ();
		  octave_idx_type b_nc = b.cols ();
		  retval = SparseComplexMatrix (nr, b_nc, x_nz);
		  retval.xcidx(0) = 0;
		  volatile octave_idx_type ii = 0;

		  OCTAVE_LOCAL_BUFFER (Complex, work, nr);

		  for (volatile octave_idx_type j = 0; j < b_nc; j++)
		    {
		      for (octave_idx_type i = 0; i < nr; i++)
			work[i] = 0.;
		      for (octave_idx_type i = b.cidx(j); i < b.cidx(j+1); i++)
			work[b.ridx(i)] = b.data(i);

		      F77_XFCN (zgttrs, ZGTTRS, 
				(F77_CONST_CHAR_ARG2 (&job, 1),
				 nr, 1, DL, D, DU, DU2, pipvt, 
				 work, b.rows (), err
				 F77_CHAR_ARG_LEN (1)));
		    
		      if (f77_exception_encountered)
			{
			  (*current_liboctave_error_handler)
			    ("unrecoverable error in zgttrs");
			  break;
			}

		      // Count non-zeros in work vector and adjust 
		      // space in retval if needed
		      octave_idx_type new_nnz = 0;
		      for (octave_idx_type i = 0; i < nr; i++)
			if (work[i] != 0.)
			  new_nnz++;

		      if (ii + new_nnz > x_nz)
			{
			  // Resize the sparse matrix
			  octave_idx_type sz = new_nnz * (b_nc - j) + x_nz;
			  retval.change_capacity (sz);
			  x_nz = sz;
			}

		      for (octave_idx_type i = 0; i < nr; i++)
			if (work[i] != 0.)
			  {
			    retval.xridx(ii) = i;
			    retval.xdata(ii++) = work[i];
			  }
		      retval.xcidx(j+1) = ii;
		    }

		  retval.maybe_compress ();
		}
	    }
	}
      else if (typ != SparseType::Tridiagonal_Hermitian)
	(*current_liboctave_error_handler) ("incorrect matrix type");
    }

  return retval;
}

ComplexMatrix
SparseComplexMatrix::trisolve (SparseType &mattype, const ComplexMatrix& b,
			       octave_idx_type& err, double& rcond, 
			       solve_singularity_handler sing_handler) const
{
  ComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  err = 0;

  if (nr == 0 || nc == 0 || nr != nc || nr != b.rows ())
    (*current_liboctave_error_handler)
      ("matrix dimension mismatch solution of linear equations");
  else
    {
      // Print spparms("spumoni") info if requested
      volatile int typ = mattype.type ();
      mattype.info ();
      
      if (typ == SparseType::Tridiagonal_Hermitian)
	{
	  OCTAVE_LOCAL_BUFFER (double, D, nr);
	  OCTAVE_LOCAL_BUFFER (Complex, DL, nr - 1);

	  if (mattype.is_dense ())
	    {
	      octave_idx_type ii = 0;

	      for (octave_idx_type j = 0; j < nc-1; j++)
		{
		  D[j] = std::real(data(ii++));
		  DL[j] = data(ii);
		  ii += 2;
		}
	      D[nc-1] = std::real(data(ii));
	    }
	  else
	    {
	      D[0] = 0.;
	      for (octave_idx_type i = 0; i < nr - 1; i++)
		{
		  D[i+1] = 0.;
		  DL[i] = 0.;
		}

	      for (octave_idx_type j = 0; j < nc; j++)
		for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
		  {
		    if (ridx(i) == j)
		      D[j] = std::real (data(i));
		    else if (ridx(i) == j + 1)
		      DL[j] = data(i);
		  }
	    }

	  octave_idx_type b_nr = b.rows ();
	  octave_idx_type b_nc = b.cols();
	  rcond = 1.;

	  retval = ComplexMatrix (b);
	  Complex *result = retval.fortran_vec ();
		  
	  F77_XFCN (zptsv, ZPTSV, (nr, b_nc, D, DL, result, 
				   b_nr, err));

	  if (f77_exception_encountered)
	    {
	      (*current_liboctave_error_handler) 
		("unrecoverable error in zptsv");
	      err = -1;
	    }
	  else if (err != 0)
	    {
	      err = 0;
	      mattype.mark_as_unsymmetric ();
	      typ = SparseType::Tridiagonal;
	    }
	}

      if (typ == SparseType::Tridiagonal)
	{
	  OCTAVE_LOCAL_BUFFER (Complex, DU, nr - 1);
	  OCTAVE_LOCAL_BUFFER (Complex, D, nr);
	  OCTAVE_LOCAL_BUFFER (Complex, DL, nr - 1);

	  if (mattype.is_dense ())
	    {
	      octave_idx_type ii = 0;

	      for (octave_idx_type j = 0; j < nc-1; j++)
		{
		  D[j] = data(ii++);
		  DL[j] = data(ii++);
		  DU[j] = data(ii++);
		}
	      D[nc-1] = data(ii);
	    }
	  else
	    {
	      D[0] = 0.;
	      for (octave_idx_type i = 0; i < nr - 1; i++)
		{
		  D[i+1] = 0.;
		  DL[i] = 0.;
		  DU[i] = 0.;
		}

	      for (octave_idx_type j = 0; j < nc; j++)
		for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
		  {
		    if (ridx(i) == j)
		      D[j] = data(i);
		    else if (ridx(i) == j + 1)
		      DL[j] = data(i);
		    else if (ridx(i) == j - 1)
		      DU[j-1] = data(i);
		  }
	    }

	  octave_idx_type b_nr = b.rows();
	  octave_idx_type b_nc = b.cols();
	  rcond = 1.;

	  retval = ComplexMatrix (b);
	  Complex *result = retval.fortran_vec ();
	      
	  F77_XFCN (zgtsv, ZGTSV, (nr, b_nc, DL, D, DU, result, 
				   b_nr, err));

	  if (f77_exception_encountered)
	    {
	      (*current_liboctave_error_handler) 
		("unrecoverable error in zgtsv");
	      err = -1;
	    }
	  else if (err != 0)
	    {
	      rcond = 0.;
	      err = -2;
		      
	      if (sing_handler)
		sing_handler (rcond);
	      else
		(*current_liboctave_error_handler)
		  ("matrix singular to machine precision");
	    }
	}
      else if (typ != SparseType::Tridiagonal_Hermitian)
	(*current_liboctave_error_handler) ("incorrect matrix type");
    }

  return retval;
}

SparseComplexMatrix
SparseComplexMatrix::trisolve (SparseType &mattype, 
		     const SparseComplexMatrix& b, octave_idx_type& err, double& rcond, 
		     solve_singularity_handler sing_handler) const
{
  SparseComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  err = 0;

  if (nr == 0 || nc == 0 || nr != nc || nr != b.rows ())
    (*current_liboctave_error_handler)
      ("matrix dimension mismatch solution of linear equations");
  else
    {
      // Print spparms("spumoni") info if requested
      int typ = mattype.type ();
      mattype.info ();
      
      // Note can't treat symmetric case as there is no dpttrf function
      if (typ == SparseType::Tridiagonal ||
	  typ == SparseType::Tridiagonal_Hermitian)
	{
	  OCTAVE_LOCAL_BUFFER (Complex, DU2, nr - 2);
	  OCTAVE_LOCAL_BUFFER (Complex, DU, nr - 1);
	  OCTAVE_LOCAL_BUFFER (Complex, D, nr);
	  OCTAVE_LOCAL_BUFFER (Complex, DL, nr - 1);
	  Array<octave_idx_type> ipvt (nr);
	  octave_idx_type *pipvt = ipvt.fortran_vec ();

	  if (mattype.is_dense ())
	    {
	      octave_idx_type ii = 0;

	      for (octave_idx_type j = 0; j < nc-1; j++)
		{
		  D[j] = data(ii++);
		  DL[j] = data(ii++);
		  DU[j] = data(ii++);
		}
	      D[nc-1] = data(ii);
	    }
	  else
	    {
	      D[0] = 0.;
	      for (octave_idx_type i = 0; i < nr - 1; i++)
		{
		  D[i+1] = 0.;
		  DL[i] = 0.;
		  DU[i] = 0.;
		}

	      for (octave_idx_type j = 0; j < nc; j++)
		for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
		  {
		    if (ridx(i) == j)
		      D[j] = data(i);
		    else if (ridx(i) == j + 1)
		      DL[j] = data(i);
		    else if (ridx(i) == j - 1)
		      DU[j-1] = data(i);
		  }
	    }

	  F77_XFCN (zgttrf, ZGTTRF, (nr, DL, D, DU, DU2, pipvt, err));

	  if (f77_exception_encountered)
	    (*current_liboctave_error_handler) 
	      ("unrecoverable error in zgttrf");
	  else
	    {
	      rcond = 0.0;
	      if (err != 0) 
		{
		  err = -2;

		  if (sing_handler)
		    sing_handler (rcond);
		  else
		    (*current_liboctave_error_handler)
		      ("matrix singular to machine precision");
		} 
	      else 
		{	
		  rcond = 1.;
		  char job = 'N';
		  octave_idx_type b_nr = b.rows ();
		  octave_idx_type b_nc = b.cols ();
		  OCTAVE_LOCAL_BUFFER (Complex, Bx, b_nr);

		  // Take a first guess that the number of non-zero terms
		  // will be as many as in b
		  volatile octave_idx_type x_nz = b.nnz ();
		  volatile octave_idx_type ii = 0;
		  retval = SparseComplexMatrix (b_nr, b_nc, x_nz);

		  retval.xcidx(0) = 0;
		  for (volatile octave_idx_type j = 0; j < b_nc; j++)
		    {

		      for (octave_idx_type i = 0; i < b_nr; i++)
			Bx[i] = b (i,j);

		      F77_XFCN (zgttrs, ZGTTRS, 
				(F77_CONST_CHAR_ARG2 (&job, 1),
				 nr, 1, DL, D, DU, DU2, pipvt, 
				 Bx, b_nr, err
				 F77_CHAR_ARG_LEN (1)));
		    
		      if (f77_exception_encountered)
			{
			  (*current_liboctave_error_handler)
			    ("unrecoverable error in zgttrs");
			  break;
			}

		      if (err != 0)
			{
			  (*current_liboctave_error_handler)
			    ("SparseComplexMatrix::solve solve failed");

			  err = -1;
			  break;
			}

		      // Count non-zeros in work vector and adjust 
		      // space in retval if needed
		      octave_idx_type new_nnz = 0;
		      for (octave_idx_type i = 0; i < nr; i++)
			if (Bx[i] != 0.)
			  new_nnz++;
		      
		      if (ii + new_nnz > x_nz)
			{
			  // Resize the sparse matrix
			  octave_idx_type sz = new_nnz * (b_nc - j) + x_nz;
			  retval.change_capacity (sz);
			  x_nz = sz;
			}
			  
		      for (octave_idx_type i = 0; i < nr; i++)
			if (Bx[i] != 0.)
			  {
			    retval.xridx(ii) = i;
			    retval.xdata(ii++) = Bx[i];
			  }

		      retval.xcidx(j+1) = ii;
		    }

		  retval.maybe_compress ();
		}
	    }
	}
      else if (typ != SparseType::Tridiagonal_Hermitian)
	(*current_liboctave_error_handler) ("incorrect matrix type");
    }

  return retval;
}

ComplexMatrix
SparseComplexMatrix::bsolve (SparseType &mattype, const Matrix& b, octave_idx_type& err, 
			     double& rcond,
			     solve_singularity_handler sing_handler) const
{
  ComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  err = 0;

  if (nr == 0 || nc == 0 || nr != nc || nr != b.rows ())
    (*current_liboctave_error_handler)
      ("matrix dimension mismatch solution of linear equations");
  else
    {
      // Print spparms("spumoni") info if requested
      volatile int typ = mattype.type ();
      mattype.info ();

      if (typ == SparseType::Banded_Hermitian)
	{
	  octave_idx_type n_lower = mattype.nlower ();
	  octave_idx_type ldm = n_lower + 1;
	  ComplexMatrix m_band (ldm, nc);
	  Complex *tmp_data = m_band.fortran_vec ();
	      
	  if (! mattype.is_dense ()) 
	    {
	      octave_idx_type ii = 0;

	      for (octave_idx_type j = 0; j < ldm; j++)
		for (octave_idx_type i = 0; i < nc; i++)
		  tmp_data[ii++] = 0.;
	    }

	  for (octave_idx_type j = 0; j < nc; j++)
	    for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
	      {
		octave_idx_type ri = ridx (i);
		if (ri >= j)
		  m_band(ri - j, j) = data(i);
	      }

	  // Calculate the norm of the matrix, for later use.
	  // double anorm = m_band.abs().sum().row(0).max();

	  char job = 'L';
	  F77_XFCN (zpbtrf, ZPBTRF, (F77_CONST_CHAR_ARG2 (&job, 1),
				     nr, n_lower, tmp_data, ldm, err
				     F77_CHAR_ARG_LEN (1)));
	    
	  if (f77_exception_encountered)
	    (*current_liboctave_error_handler) 
	      ("unrecoverable error in zpbtrf");
	  else
	    {
	      rcond = 0.0;
	      if (err != 0) 
		{
		  // Matrix is not positive definite!! Fall through to
		  // unsymmetric banded solver.
		  mattype.mark_as_unsymmetric ();
		  typ = SparseType::Banded;
		  err = 0;
		} 
	      else 
		{
		  // Unfortunately, the time to calculate the condition
		  // number is dominant for narrow banded matrices and
		  // so we rely on the "err" flag from xPBTRF to flag
		  // singularity. The commented code below is left here
		  // for reference

		  //Array<double> z (3 * nr);
		  //Complex *pz = z.fortran_vec ();
		  //Array<octave_idx_type> iz (nr);
		  //octave_idx_type *piz = iz.fortran_vec ();
		  //
		  //F77_XFCN (zpbcon, ZGBCON, 
		  //	(F77_CONST_CHAR_ARG2 (&job, 1),
		  //	 nr, n_lower, tmp_data, ldm,
		  //	 anorm, rcond, pz, piz, err
		  //	 F77_CHAR_ARG_LEN (1)));
		  //
		  //
		  //if (f77_exception_encountered)
		  //	(*current_liboctave_error_handler) 
		  //	  ("unrecoverable error in zpbcon");
		  //
		  //if (err != 0) 
		  //	err = -2;
		  //
		  //volatile double rcond_plus_one = rcond + 1.0;
		  //
		  //if (rcond_plus_one == 1.0 || xisnan (rcond))
		  //  {
		  //    err = -2;
		  //
		  //    if (sing_handler)
		  //      sing_handler (rcond);
		  //    else
		  //      (*current_liboctave_error_handler)
		  //        ("matrix singular to machine precision, rcond = %g",
		  //         rcond);
		  //  }
		  //else
		  //    REST OF CODE, EXCEPT rcond=1

		  rcond = 1.;
		  retval = ComplexMatrix (b);
		  Complex *result = retval.fortran_vec ();

		  octave_idx_type b_nc = b.cols ();

		  F77_XFCN (zpbtrs, ZPBTRS, 
			    (F77_CONST_CHAR_ARG2 (&job, 1),
			     nr, n_lower, b_nc, tmp_data,
			     ldm, result, b.rows(), err
			     F77_CHAR_ARG_LEN (1)));
		    
		  if (f77_exception_encountered)
		    (*current_liboctave_error_handler)
		      ("unrecoverable error in zpbtrs");

		  if (err != 0)
		    {
		      (*current_liboctave_error_handler) 
			("SparseMatrix::solve solve failed");
		      err = -1;
		    }
		}
	    }
	}

      if (typ == SparseType::Banded)
	{
	  // Create the storage for the banded form of the sparse matrix
	  octave_idx_type n_upper = mattype.nupper ();
	  octave_idx_type n_lower = mattype.nlower ();
	  octave_idx_type ldm = n_upper + 2 * n_lower + 1;

	  ComplexMatrix m_band (ldm, nc);
	  Complex *tmp_data = m_band.fortran_vec ();
	      
	  if (! mattype.is_dense ()) 
	    {
	      octave_idx_type ii = 0;

	      for (octave_idx_type j = 0; j < ldm; j++)
		for (octave_idx_type i = 0; i < nc; i++)
		  tmp_data[ii++] = 0.;
	    }

	  for (octave_idx_type j = 0; j < nc; j++)
	    for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
	      m_band(ridx(i) - j + n_lower + n_upper, j) = data(i);

	  Array<octave_idx_type> ipvt (nr);
	  octave_idx_type *pipvt = ipvt.fortran_vec ();

	  F77_XFCN (zgbtrf, ZGBTRF, (nr, nr, n_lower, n_upper, tmp_data, 
				     ldm, pipvt, err));
	    
	  if (f77_exception_encountered)
	    (*current_liboctave_error_handler) 
	      ("unrecoverable error in zgbtrf");
	  else
	    {
	      // Throw-away extra info LAPACK gives so as to not 
	      // change output.
	      rcond = 0.0;
	      if (err != 0) 
		{
		  err = -2;

		  if (sing_handler)
		    sing_handler (rcond);
		  else
		    (*current_liboctave_error_handler)
		      ("matrix singular to machine precision");

		} 
	      else 
		{
		  char job = '1';

		  // Unfortunately, the time to calculate the condition
		  // number is dominant for narrow banded matrices and
		  // so we rely on the "err" flag from xPBTRF to flag
		  // singularity. The commented code below is left here
		  // for reference

		  //F77_XFCN (zgbcon, ZGBCON, 
		  //	(F77_CONST_CHAR_ARG2 (&job, 1),
		  //	 nc, n_lower, n_upper, tmp_data, ldm, pipvt,
		  //	 anorm, rcond, pz, piz, err
		  //	 F77_CHAR_ARG_LEN (1)));
		  //
		  //if (f77_exception_encountered)
		  //  (*current_liboctave_error_handler) 
		  //    ("unrecoverable error in zgbcon");
		  //
		  // if (err != 0) 
		  //  err = -2;
		  //
		  //volatile double rcond_plus_one = rcond + 1.0;
		  //
		  //if (rcond_plus_one == 1.0 || xisnan (rcond))
		  //  {
		  //    err = -2;
		  //
		  //    if (sing_handler)
		  //      sing_handler (rcond);
		  //    else
		  //      (*current_liboctave_error_handler)
		  //        ("matrix singular to machine precision, rcond = %g",
		  //         rcond);
		  //  }
		  //else
		  //  REST OF CODE, EXCEPT rcond=1

		  rcond = 1.;
		  retval = ComplexMatrix (b);
		  Complex *result = retval.fortran_vec ();

		  octave_idx_type b_nc = b.cols ();

		  job = 'N';
		  F77_XFCN (zgbtrs, ZGBTRS, 
			    (F77_CONST_CHAR_ARG2 (&job, 1),
			     nr, n_lower, n_upper, b_nc, tmp_data,
			     ldm, pipvt, result, b.rows(), err
			     F77_CHAR_ARG_LEN (1)));
		    
		  if (f77_exception_encountered)
		    (*current_liboctave_error_handler)
		      ("unrecoverable error in zgbtrs");
		}
	    }
	}
      else if (typ != SparseType::Banded_Hermitian)
	(*current_liboctave_error_handler) ("incorrect matrix type");
    }

  return retval;
}

SparseComplexMatrix
SparseComplexMatrix::bsolve (SparseType &mattype, const SparseMatrix& b,
			     octave_idx_type& err, double& rcond, 
			     solve_singularity_handler sing_handler) const
{
  SparseComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  err = 0;

  if (nr == 0 || nc == 0 || nr != nc || nr != b.rows ())
    (*current_liboctave_error_handler)
      ("matrix dimension mismatch solution of linear equations");
  else
    {
      // Print spparms("spumoni") info if requested
      volatile int typ = mattype.type ();
      mattype.info ();

      if (typ == SparseType::Banded_Hermitian)
	{
	  octave_idx_type n_lower = mattype.nlower ();
	  octave_idx_type ldm = n_lower + 1;

	  ComplexMatrix m_band (ldm, nc);
	  Complex *tmp_data = m_band.fortran_vec ();
	      
	  if (! mattype.is_dense ()) 
	    {
	      octave_idx_type ii = 0;

	      for (octave_idx_type j = 0; j < ldm; j++)
		for (octave_idx_type i = 0; i < nc; i++)
		  tmp_data[ii++] = 0.;
	    }

	  for (octave_idx_type j = 0; j < nc; j++)
	    for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
	      {
		octave_idx_type ri = ridx (i);
		if (ri >= j)
		  m_band(ri - j, j) = data(i);
	      }

	  char job = 'L';
	  F77_XFCN (zpbtrf, ZPBTRF, (F77_CONST_CHAR_ARG2 (&job, 1),
				     nr, n_lower, tmp_data, ldm, err
				     F77_CHAR_ARG_LEN (1)));
	    
	  if (f77_exception_encountered)
	    (*current_liboctave_error_handler) 
	      ("unrecoverable error in zpbtrf");
	  else
	    {
	      rcond = 0.0;
	      if (err != 0) 
		{
		  mattype.mark_as_unsymmetric ();
		  typ = SparseType::Banded;
		  err = 0;
		} 
	      else 
		{
		  rcond = 1.;
		  octave_idx_type b_nr = b.rows ();
		  octave_idx_type b_nc = b.cols ();
		  OCTAVE_LOCAL_BUFFER (Complex, Bx, b_nr);

		  // Take a first guess that the number of non-zero terms
		  // will be as many as in b
		  volatile octave_idx_type x_nz = b.nnz ();
		  volatile octave_idx_type ii = 0;
		  retval = SparseComplexMatrix (b_nr, b_nc, x_nz);

		  retval.xcidx(0) = 0;
		  for (volatile octave_idx_type j = 0; j < b_nc; j++)
		    {
		      for (octave_idx_type i = 0; i < b_nr; i++)
			Bx[i] = b.elem (i, j);

		      F77_XFCN (zpbtrs, ZPBTRS, 
				(F77_CONST_CHAR_ARG2 (&job, 1),
				 nr, n_lower, 1, tmp_data,
				 ldm, Bx, b_nr, err
				 F77_CHAR_ARG_LEN (1)));
		    
		      if (f77_exception_encountered)
			{
			  (*current_liboctave_error_handler)
			    ("unrecoverable error in dpbtrs");
			  err = -1;
			  break;
			}

		      if (err != 0)
			{
			  (*current_liboctave_error_handler) 
			    ("SparseComplexMatrix::solve solve failed");
			  err = -1;
			  break;
			}

		      for (octave_idx_type i = 0; i < b_nr; i++)
			{
			  Complex tmp = Bx[i];
			  if (tmp != 0.0)
			    {
			      if (ii == x_nz)
				{
				  // Resize the sparse matrix
				  octave_idx_type sz = x_nz * (b_nc - j) / b_nc;
				  sz = (sz > 10 ? sz : 10) + x_nz;
				  retval.change_capacity (sz);
				  x_nz = sz;
				}
			      retval.xdata(ii) = tmp;
			      retval.xridx(ii++) = i;
			    }
			}
		      retval.xcidx(j+1) = ii;
		    }

		  retval.maybe_compress ();
		}
	    }
	}

      if (typ == SparseType::Banded)
	{
	  // Create the storage for the banded form of the sparse matrix
	  octave_idx_type n_upper = mattype.nupper ();
	  octave_idx_type n_lower = mattype.nlower ();
	  octave_idx_type ldm = n_upper + 2 * n_lower + 1;

	  ComplexMatrix m_band (ldm, nc);
	  Complex *tmp_data = m_band.fortran_vec ();
	      
	  if (! mattype.is_dense ()) 
	    {
	      octave_idx_type ii = 0;

	      for (octave_idx_type j = 0; j < ldm; j++)
		for (octave_idx_type i = 0; i < nc; i++)
		  tmp_data[ii++] = 0.;
	    }

	  for (octave_idx_type j = 0; j < nc; j++)
	    for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
	      m_band(ridx(i) - j + n_lower + n_upper, j) = data(i);

	  Array<octave_idx_type> ipvt (nr);
	  octave_idx_type *pipvt = ipvt.fortran_vec ();

	  F77_XFCN (zgbtrf, ZGBTRF, (nr, nr, n_lower, n_upper, tmp_data, 
				     ldm, pipvt, err));
	    
	  if (f77_exception_encountered)
	    (*current_liboctave_error_handler) 
	      ("unrecoverable error in zgbtrf");
	  else
	    {
	      rcond = 0.0;
	      if (err != 0) 
		{
		  err = -2;

		  if (sing_handler)
		    sing_handler (rcond);
		  else
		    (*current_liboctave_error_handler)
		      ("matrix singular to machine precision");

		} 
	      else 
		{
		  char job = 'N';
		  volatile octave_idx_type x_nz = b.nnz ();
		  octave_idx_type b_nc = b.cols ();
		  retval = SparseComplexMatrix (nr, b_nc, x_nz);
		  retval.xcidx(0) = 0;
		  volatile octave_idx_type ii = 0;

		  OCTAVE_LOCAL_BUFFER (Complex, work, nr);

		  for (volatile octave_idx_type j = 0; j < b_nc; j++)
		    {
		      for (octave_idx_type i = 0; i < nr; i++)
			work[i] = 0.;
		      for (octave_idx_type i = b.cidx(j); i < b.cidx(j+1); i++)
			work[b.ridx(i)] = b.data(i);

		      F77_XFCN (zgbtrs, ZGBTRS, 
				(F77_CONST_CHAR_ARG2 (&job, 1),
				 nr, n_lower, n_upper, 1, tmp_data,
				 ldm, pipvt, work, b.rows (), err
				 F77_CHAR_ARG_LEN (1)));
		    
		      if (f77_exception_encountered)
			{
			  (*current_liboctave_error_handler)
			    ("unrecoverable error in zgbtrs");
			  break;
			}

		      // Count non-zeros in work vector and adjust 
		      // space in retval if needed
		      octave_idx_type new_nnz = 0;
		      for (octave_idx_type i = 0; i < nr; i++)
			if (work[i] != 0.)
			  new_nnz++;

		      if (ii + new_nnz > x_nz)
			{
			  // Resize the sparse matrix
			  octave_idx_type sz = new_nnz * (b_nc - j) + x_nz;
			  retval.change_capacity (sz);
			  x_nz = sz;
			}

		      for (octave_idx_type i = 0; i < nr; i++)
			if (work[i] != 0.)
			  {
			    retval.xridx(ii) = i;
			    retval.xdata(ii++) = work[i];
			  }
		      retval.xcidx(j+1) = ii;
		    }

		  retval.maybe_compress ();
		}
	    }
	}
      else if (typ != SparseType::Banded_Hermitian)
	(*current_liboctave_error_handler) ("incorrect matrix type");
    }

  return retval;
}

ComplexMatrix
SparseComplexMatrix::bsolve (SparseType &mattype, const ComplexMatrix& b, 
			     octave_idx_type& err, double& rcond, 
			     solve_singularity_handler sing_handler) const
{
  ComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  err = 0;

  if (nr == 0 || nc == 0 || nr != nc || nr != b.rows ())
    (*current_liboctave_error_handler)
      ("matrix dimension mismatch solution of linear equations");
  else
    {
      // Print spparms("spumoni") info if requested
      volatile int typ = mattype.type ();
      mattype.info ();

      if (typ == SparseType::Banded_Hermitian)
	{
	  octave_idx_type n_lower = mattype.nlower ();
	  octave_idx_type ldm = n_lower + 1;

	  ComplexMatrix m_band (ldm, nc);
	  Complex *tmp_data = m_band.fortran_vec ();
	      
	  if (! mattype.is_dense ()) 
	    {
	      octave_idx_type ii = 0;

	      for (octave_idx_type j = 0; j < ldm; j++)
		for (octave_idx_type i = 0; i < nc; i++)
		  tmp_data[ii++] = 0.;
	    }

	  for (octave_idx_type j = 0; j < nc; j++)
	    for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
	      {
		octave_idx_type ri = ridx (i);
		if (ri >= j)
		  m_band(ri - j, j) = data(i);
	      }

	  char job = 'L';
	  F77_XFCN (zpbtrf, ZPBTRF, (F77_CONST_CHAR_ARG2 (&job, 1),
				     nr, n_lower, tmp_data, ldm, err
				     F77_CHAR_ARG_LEN (1)));
	    
	  if (f77_exception_encountered)
	    (*current_liboctave_error_handler) 
	      ("unrecoverable error in zpbtrf");
	  else
	    {
	      rcond = 0.0;
	      if (err != 0) 
		{
		  // Matrix is not positive definite!! Fall through to
		  // unsymmetric banded solver.
		  mattype.mark_as_unsymmetric ();
		  typ = SparseType::Banded;
		  err = 0;
		} 
	      else 
		{
		  rcond = 1.;
		  octave_idx_type b_nr = b.rows ();
		  octave_idx_type b_nc = b.cols ();
		  retval = ComplexMatrix (b);
		  Complex *result = retval.fortran_vec ();

		  F77_XFCN (zpbtrs, ZPBTRS, 
			    (F77_CONST_CHAR_ARG2 (&job, 1),
			     nr, n_lower, b_nc, tmp_data,
			     ldm, result, b_nr, err
			     F77_CHAR_ARG_LEN (1)));
		    
		  if (f77_exception_encountered)
		    {
		      (*current_liboctave_error_handler)
			("unrecoverable error in zpbtrs");
		      err = -1;
		    }

		  if (err != 0)
		    {
		      (*current_liboctave_error_handler) 
			("SparseComplexMatrix::solve solve failed");
		      err = -1;
		    }
		}
	    }
	}

      if (typ == SparseType::Banded)
	{
	  // Create the storage for the banded form of the sparse matrix
	  octave_idx_type n_upper = mattype.nupper ();
	  octave_idx_type n_lower = mattype.nlower ();
	  octave_idx_type ldm = n_upper + 2 * n_lower + 1;

	  ComplexMatrix m_band (ldm, nc);
	  Complex *tmp_data = m_band.fortran_vec ();
	      
	  if (! mattype.is_dense ()) 
	    {
	      octave_idx_type ii = 0;

	      for (octave_idx_type j = 0; j < ldm; j++)
		for (octave_idx_type i = 0; i < nc; i++)
		  tmp_data[ii++] = 0.;
	    }

	  for (octave_idx_type j = 0; j < nc; j++)
	    for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
	      m_band(ridx(i) - j + n_lower + n_upper, j) = data(i);

	  Array<octave_idx_type> ipvt (nr);
	  octave_idx_type *pipvt = ipvt.fortran_vec ();

	  F77_XFCN (zgbtrf, ZGBTRF, (nr, nr, n_lower, n_upper, tmp_data, 
				     ldm, pipvt, err));
	    
	  if (f77_exception_encountered)
	    (*current_liboctave_error_handler) 
	      ("unrecoverable error in zgbtrf");
	  else
	    {
	      rcond = 0.0;
	      if (err != 0) 
		{
		  err = -2;

		  if (sing_handler)
		    sing_handler (rcond);
		  else
		    (*current_liboctave_error_handler)
		      ("matrix singular to machine precision");

		} 
	      else 
		{
		  char job = 'N';
		  octave_idx_type b_nc = b.cols ();
		  retval = ComplexMatrix (b);
		  Complex *result = retval.fortran_vec ();

		  F77_XFCN (zgbtrs, ZGBTRS, 
			    (F77_CONST_CHAR_ARG2 (&job, 1),
			     nr, n_lower, n_upper, b_nc, tmp_data,
			     ldm, pipvt, result, b.rows (), err
			     F77_CHAR_ARG_LEN (1)));
		    
		  if (f77_exception_encountered)
		    {
		      (*current_liboctave_error_handler)
			("unrecoverable error in dgbtrs");
		    }
		}
	    }
	}
      else if (typ != SparseType::Banded_Hermitian)
	(*current_liboctave_error_handler) ("incorrect matrix type");
    }

  return retval;
}

SparseComplexMatrix
SparseComplexMatrix::bsolve (SparseType &mattype, const SparseComplexMatrix& b,
		     octave_idx_type& err, double& rcond, 
		     solve_singularity_handler sing_handler) const
{
  SparseComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  err = 0;

  if (nr == 0 || nc == 0 || nr != nc || nr != b.rows ())
    (*current_liboctave_error_handler)
      ("matrix dimension mismatch solution of linear equations");
  else
    {
      // Print spparms("spumoni") info if requested
      volatile int typ = mattype.type ();
      mattype.info ();

      if (typ == SparseType::Banded_Hermitian)
	{
	  octave_idx_type n_lower = mattype.nlower ();
	  octave_idx_type ldm = n_lower + 1;

	  ComplexMatrix m_band (ldm, nc);
	  Complex *tmp_data = m_band.fortran_vec ();
	      
	  if (! mattype.is_dense ()) 
	    {
	      octave_idx_type ii = 0;

	      for (octave_idx_type j = 0; j < ldm; j++)
		for (octave_idx_type i = 0; i < nc; i++)
		  tmp_data[ii++] = 0.;
	    }

	  for (octave_idx_type j = 0; j < nc; j++)
	    for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
	      {
		octave_idx_type ri = ridx (i);
		if (ri >= j)
		  m_band(ri - j, j) = data(i);
	      }

	  char job = 'L';
	  F77_XFCN (zpbtrf, ZPBTRF, (F77_CONST_CHAR_ARG2 (&job, 1),
				     nr, n_lower, tmp_data, ldm, err
				     F77_CHAR_ARG_LEN (1)));
	    
	  if (f77_exception_encountered)
	    (*current_liboctave_error_handler) 
	      ("unrecoverable error in zpbtrf");
	  else
	    {
	      rcond = 0.0;
	      if (err != 0) 
		{
		  // Matrix is not positive definite!! Fall through to
		  // unsymmetric banded solver.
		  mattype.mark_as_unsymmetric ();
		  typ = SparseType::Banded;

		  err = 0;
		} 
	      else 
		{
		  rcond = 1.;
		  octave_idx_type b_nr = b.rows ();
		  octave_idx_type b_nc = b.cols ();
		  OCTAVE_LOCAL_BUFFER (Complex, Bx, b_nr);

		  // Take a first guess that the number of non-zero terms
		  // will be as many as in b
		  volatile octave_idx_type x_nz = b.nnz ();
		  volatile octave_idx_type ii = 0;
		  retval = SparseComplexMatrix (b_nr, b_nc, x_nz);

		  retval.xcidx(0) = 0;
		  for (volatile octave_idx_type j = 0; j < b_nc; j++)
		    {

		      for (octave_idx_type i = 0; i < b_nr; i++)
			Bx[i] = b (i,j);

		      F77_XFCN (zpbtrs, ZPBTRS, 
				(F77_CONST_CHAR_ARG2 (&job, 1),
				 nr, n_lower, 1, tmp_data,
				 ldm, Bx, b_nr, err
				 F77_CHAR_ARG_LEN (1)));
		    
		      if (f77_exception_encountered)
			{
			  (*current_liboctave_error_handler)
			    ("unrecoverable error in zpbtrs");
			  err = -1;
			  break;
			}

		      if (err != 0)
			{
			  (*current_liboctave_error_handler) 
			    ("SparseMatrix::solve solve failed");
			  err = -1;
			  break;
			}


		      // Count non-zeros in work vector and adjust 
		      // space in retval if needed
		      octave_idx_type new_nnz = 0;
		      for (octave_idx_type i = 0; i < nr; i++)
			if (Bx[i] != 0.)
			  new_nnz++;
			  
		      if (ii + new_nnz > x_nz)
			{
			  // Resize the sparse matrix
			  octave_idx_type sz = new_nnz * (b_nc - j) + x_nz;
			  retval.change_capacity (sz);
			  x_nz = sz;
			}
			  
		      for (octave_idx_type i = 0; i < nr; i++)
			if (Bx[i] != 0.)
			  {
			    retval.xridx(ii) = i;
			    retval.xdata(ii++) = Bx[i];
			  }

		      retval.xcidx(j+1) = ii;
		    }

		  retval.maybe_compress ();
		}
	    }
	}

      if (typ == SparseType::Banded)
	{
	  // Create the storage for the banded form of the sparse matrix
	  octave_idx_type n_upper = mattype.nupper ();
	  octave_idx_type n_lower = mattype.nlower ();
	  octave_idx_type ldm = n_upper + 2 * n_lower + 1;

	  ComplexMatrix m_band (ldm, nc);
	  Complex *tmp_data = m_band.fortran_vec ();
	      
	  if (! mattype.is_dense ()) 
	    {
	      octave_idx_type ii = 0;

	      for (octave_idx_type j = 0; j < ldm; j++)
		for (octave_idx_type i = 0; i < nc; i++)
		  tmp_data[ii++] = 0.;
	    }

	  for (octave_idx_type j = 0; j < nc; j++)
	    for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
	      m_band(ridx(i) - j + n_lower + n_upper, j) = data(i);

	  Array<octave_idx_type> ipvt (nr);
	  octave_idx_type *pipvt = ipvt.fortran_vec ();

	  F77_XFCN (zgbtrf, ZGBTRF, (nr, nr, n_lower, n_upper, tmp_data, 
				     ldm, pipvt, err));
	    
	  if (f77_exception_encountered)
	    (*current_liboctave_error_handler) 
	      ("unrecoverable error in xgbtrf");
	  else
	    {
	      rcond = 0.0;
	      if (err != 0) 
		{
		  err = -2;

		  if (sing_handler)
		    sing_handler (rcond);
		  else
		    (*current_liboctave_error_handler)
		      ("matrix singular to machine precision");

		}
	      else 
		{
		  char job = 'N';
		  volatile octave_idx_type x_nz = b.nnz ();
		  octave_idx_type b_nc = b.cols ();
		  retval = SparseComplexMatrix (nr, b_nc, x_nz);
		  retval.xcidx(0) = 0;
		  volatile octave_idx_type ii = 0;

		  OCTAVE_LOCAL_BUFFER (Complex, Bx, nr);

		  for (volatile octave_idx_type j = 0; j < b_nc; j++)
		    {
		      for (octave_idx_type i = 0; i < nr; i++)
			Bx[i] = 0.;

		      for (octave_idx_type i = b.cidx(j); i < b.cidx(j+1); i++)
			Bx[b.ridx(i)] = b.data(i);

		      F77_XFCN (zgbtrs, ZGBTRS, 
				(F77_CONST_CHAR_ARG2 (&job, 1),
				 nr, n_lower, n_upper, 1, tmp_data,
				 ldm, pipvt, Bx, b.rows (), err
				 F77_CHAR_ARG_LEN (1)));
		    
		      if (f77_exception_encountered)
			{
			  (*current_liboctave_error_handler)
			    ("unrecoverable error in dgbtrs");
			  break;
			}

		      // Count non-zeros in work vector and adjust 
		      // space in retval if needed
		      octave_idx_type new_nnz = 0;
		      for (octave_idx_type i = 0; i < nr; i++)
			if (Bx[i] != 0.)
			  new_nnz++;

		      if (ii + new_nnz > x_nz)
			{
			  // Resize the sparse matrix
			  octave_idx_type sz = new_nnz * (b_nc - j) + x_nz;
			  retval.change_capacity (sz);
			  x_nz = sz;
			}

		      for (octave_idx_type i = 0; i < nr; i++)
			if (Bx[i] != 0.)
			  {
			    retval.xridx(ii) = i;
			    retval.xdata(ii++) = Bx[i]; 
			  }
		      retval.xcidx(j+1) = ii;
		    }

		  retval.maybe_compress ();
		}
	    }
	}
      else if (typ != SparseType::Banded_Hermitian)
	(*current_liboctave_error_handler) ("incorrect matrix type");
    }
  
  return retval;
}

void *
SparseComplexMatrix::factorize (octave_idx_type& err, double &rcond, Matrix &Control, 
				Matrix &Info,
				solve_singularity_handler sing_handler) const
{
  // The return values
  void *Numeric = 0;
  err = 0;

#ifdef HAVE_UMFPACK
  // Setup the control parameters
  Control = Matrix (UMFPACK_CONTROL, 1);
  double *control = Control.fortran_vec ();
  UMFPACK_ZNAME (defaults) (control);

  double tmp = Voctave_sparse_controls.get_key ("spumoni");
  if (!xisnan (tmp))
    Control (UMFPACK_PRL) = tmp;
  tmp = Voctave_sparse_controls.get_key ("piv_tol");
  if (!xisnan (tmp))
    {
      Control (UMFPACK_SYM_PIVOT_TOLERANCE) = tmp;
      Control (UMFPACK_PIVOT_TOLERANCE) = tmp;
    }

  // Set whether we are allowed to modify Q or not
  tmp = Voctave_sparse_controls.get_key ("autoamd");
  if (!xisnan (tmp))
    Control (UMFPACK_FIXQ) = tmp;

  UMFPACK_ZNAME (report_control) (control);

  const octave_idx_type *Ap = cidx ();
  const octave_idx_type *Ai = ridx ();
  const Complex *Ax = data ();
  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();

  UMFPACK_ZNAME (report_matrix) (nr, nc, Ap, Ai,
			    X_CAST (const double *, Ax), NULL, 1, control);

  void *Symbolic;
  Info = Matrix (1, UMFPACK_INFO);
  double *info = Info.fortran_vec ();
  int status = UMFPACK_ZNAME (qsymbolic) (nr, nc, Ap, Ai, 
				     X_CAST (const double *, Ax), 
				     NULL, NULL, &Symbolic, control, info);

  if (status < 0)
    {
      (*current_liboctave_error_handler) 
	("SparseComplexMatrix::solve symbolic factorization failed");
      err = -1;

      UMFPACK_ZNAME (report_status) (control, status);
      UMFPACK_ZNAME (report_info) (control, info);

      UMFPACK_ZNAME (free_symbolic) (&Symbolic) ;
    }
  else
    {
      UMFPACK_ZNAME (report_symbolic) (Symbolic, control);

      status = UMFPACK_ZNAME (numeric) (Ap, Ai,
				   X_CAST (const double *, Ax), NULL, 
				   Symbolic, &Numeric, control, info) ;
      UMFPACK_ZNAME (free_symbolic) (&Symbolic) ;

#ifdef HAVE_LSSOLVE
      rcond = Info (UMFPACK_RCOND);
      volatile double rcond_plus_one = rcond + 1.0;

      if (status == UMFPACK_WARNING_singular_matrix || 
	  rcond_plus_one == 1.0 || xisnan (rcond))
	{
	  UMFPACK_ZNAME (report_numeric) (Numeric, control);

	  err = -2;

	  if (sing_handler)
	    sing_handler (rcond);
	  else
	    (*current_liboctave_error_handler)
	      ("SparseComplexMatrix::solve matrix singular to machine precision, rcond = %g",
	       rcond);

	}
      else
#endif 
	if (status < 0)
	  {
	    (*current_liboctave_error_handler) 
	      ("SparseComplexMatrix::solve numeric factorization failed");

	    UMFPACK_ZNAME (report_status) (control, status);
	    UMFPACK_ZNAME (report_info) (control, info);
	      
	    err = -1;
	  }
	else
	  {
	    UMFPACK_ZNAME (report_numeric) (Numeric, control);
	  }
    }

  if (err != 0)
    UMFPACK_ZNAME (free_numeric) (&Numeric);
#else
  (*current_liboctave_error_handler) ("UMFPACK not installed");
#endif

  return Numeric;
}

ComplexMatrix
SparseComplexMatrix::fsolve (SparseType &mattype, const Matrix& b, octave_idx_type& err, 
			     double& rcond,
			     solve_singularity_handler sing_handler) const
{
  ComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  err = 0;

  if (nr == 0 || nc == 0 || nr != nc || nr != b.rows ())
    (*current_liboctave_error_handler)
      ("matrix dimension mismatch solution of linear equations");
  else
    {
      // Print spparms("spumoni") info if requested
      volatile int typ = mattype.type ();
      mattype.info ();

      if (typ == SparseType::Hermitian)
	{
#ifdef HAVE_CHOLMOD
	  cholmod_common Common;
	  cholmod_common *cm = &Common;

	  // Setup initial parameters
	  CHOLMOD_NAME(start) (cm);
	  cm->prefer_zomplex = false;

	  double spu = Voctave_sparse_controls.get_key ("spumoni");
	  if (spu == 0.)
	    {
	      cm->print = -1;
	      cm->print_function = NULL;
	    }
	  else
	    {
	      cm->print = (int)spu + 2;
	      cm->print_function =&SparseCholPrint;
	    }

	  cm->error_handler = &SparseCholError;
	  cm->complex_divide = CHOLMOD_NAME(divcomplex);
	  cm->hypotenuse = CHOLMOD_NAME(hypot);

#ifdef HAVE_METIS
	  // METIS 4.0.1 uses malloc and free, and will terminate MATLAB if
	  // it runs out of memory.  Use CHOLMOD's memory guard for METIS, 
	  // which mxMalloc's a huge block of memory (and then immediately 
	  // mxFree's it) before calling METIS
	  cm->metis_memory = 2.0;

#if defined(METIS_VERSION)
#if (METIS_VERSION >= METIS_VER(4,0,2))
	  // METIS 4.0.2 uses function pointers for malloc and free
	  METIS_malloc = cm->malloc_memory;
	  METIS_free   = cm->free_memory;
	  // Turn off METIS memory guard.  It is not needed, because mxMalloc
	  // will safely terminate the mexFunction and free any workspace
	  // without killing all of octave.
	  cm->metis_memory   = 0.0;
#endif
#endif
#endif
	  cm->final_ll = true;

	  cholmod_sparse Astore;
	  cholmod_sparse *A = &Astore;
	  double dummy;
	  A->nrow = nr;
	  A->ncol = nc;

	  A->p = cidx();
	  A->i = ridx();
	  A->nzmax = nonzero();
	  A->packed = true;
	  A->sorted = true;
	  A->nz = NULL;
#ifdef IDX_TYPE_LONG
	  A->itype = CHOLMOD_LONG;
#else
	  A->itype = CHOLMOD_INT;
#endif
	  A->dtype = CHOLMOD_DOUBLE;
	  A->stype = 1;
	  A->xtype = CHOLMOD_COMPLEX;

	  if (nr < 1)
	    A->x = &dummy;
	  else
	    A->x = data();

	  cholmod_dense Bstore;
	  cholmod_dense *B = &Bstore;
	  B->nrow = b.rows();
	  B->ncol = b.cols();
	  B->d = B->nrow;
	  B->nzmax = B->nrow * B->ncol;
	  B->dtype = CHOLMOD_DOUBLE;
	  B->xtype = CHOLMOD_REAL;
	  if (nc < 1 || b.cols() < 1)
	    B->x = &dummy;
	  else
	    // We won't alter it, honest :-)
	    B->x = const_cast<double *>(b.fortran_vec());

	  cholmod_factor *L;
	  BEGIN_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;
	  L = CHOLMOD_NAME(analyze) (A, cm);
	  CHOLMOD_NAME(factorize) (A, L, cm);
	  rcond = CHOLMOD_NAME(rcond)(L, cm);
	  END_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;

	  if (rcond == 0.0)
	    {
	      // Either its indefinite or singular. Try UMFPACK
	      mattype.mark_as_unsymmetric ();
	      typ = SparseType::Full;
	    }
	  else
	    {
	      volatile double rcond_plus_one = rcond + 1.0;

	      if (rcond_plus_one == 1.0 || xisnan (rcond))
		{
		  err = -2;

		  if (sing_handler)
		    sing_handler (rcond);
		  else
		    (*current_liboctave_error_handler)
		      ("SparseMatrix::solve matrix singular to machine precision, rcond = %g",
		       rcond);
	      
#ifdef HAVE_LSSOLVE
		  return retval;
#endif
		}

	      cholmod_dense *X;
	      BEGIN_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;
	      X = CHOLMOD_NAME(solve) (CHOLMOD_A, L, B, cm);
	      END_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;

	      retval.resize (b.rows (), b.cols());
	      for (octave_idx_type j = 0; j < b.cols(); j++)
		{
		  octave_idx_type jr = j * b.rows();
		  for (octave_idx_type i = 0; i < b.rows(); i++)
		    retval.xelem(i,j) = static_cast<Complex *>(X->x)[jr + i];
		}

	      BEGIN_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;
	      CHOLMOD_NAME(free_dense) (&X, cm);
	      CHOLMOD_NAME(free_factor) (&L, cm);
	      CHOLMOD_NAME(finish) (cm);
	      CHOLMOD_NAME(print_common) (" ", cm);
	      END_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;
	    }
#else
	  (*current_liboctave_warning_handler)
	    ("CHOLMOD not installed");

	  mattype.mark_as_unsymmetric ();
	  typ = SparseType::Full;
#endif
	}

      if (typ == SparseType::Full)
	{
#ifdef HAVE_UMFPACK
	  Matrix Control, Info;
	  void *Numeric = factorize (err, rcond, Control, Info, 
				     sing_handler);

	  if (err == 0)
	    {
	      octave_idx_type b_nr = b.rows ();
	      octave_idx_type b_nc = b.cols ();
	      int status = 0;
	      double *control = Control.fortran_vec ();
	      double *info = Info.fortran_vec ();
	      const octave_idx_type *Ap = cidx ();
	      const octave_idx_type *Ai = ridx ();
	      const Complex *Ax = data ();
#ifdef UMFPACK_SEPARATE_SPLIT
	      const double *Bx = b.fortran_vec ();
	      OCTAVE_LOCAL_BUFFER (double, Bz, b_nr);
	      for (octave_idx_type i = 0; i < b_nr; i++)
		Bz[i] = 0.;
#else
	      OCTAVE_LOCAL_BUFFER (Complex, Bz, b_nr);
#endif
	      retval.resize (b_nr, b_nc);
	      Complex *Xx = retval.fortran_vec ();

	      for (octave_idx_type j = 0, iidx = 0; j < b_nc; j++, iidx += b_nr)
		{
#ifdef UMFPACK_SEPARATE_SPLIT
		  status = UMFPACK_ZNAME (solve) (UMFPACK_A, Ap,
					     Ai, X_CAST (const double *, Ax), 
					     NULL,
					     X_CAST (double *, &Xx[iidx]), 
					     NULL,
					     &Bx[iidx], Bz, Numeric, 
					     control, info);
#else
		  for (octave_idx_type i = 0; i < b_nr; i++)
		    Bz[i] = b.elem (i, j);

		  status = UMFPACK_ZNAME (solve) (UMFPACK_A, Ap,
					     Ai, X_CAST (const double *, Ax), 
					     NULL,
					     X_CAST (double *, &Xx[iidx]), 
					     NULL,
					     X_CAST (const double *, Bz),
					     NULL, Numeric, 
					     control, info);
#endif

		  if (status < 0)
		    {
		      (*current_liboctave_error_handler) 
			("SparseComplexMatrix::solve solve failed");

		      UMFPACK_ZNAME (report_status) (control, status);
		      
		      err = -1;

		      break;
		    }
		}

#ifndef HAVE_LSSOLVE
	      rcond = Info (UMFPACK_RCOND);
	      volatile double rcond_plus_one = rcond + 1.0;

	      if (status == UMFPACK_WARNING_singular_matrix || 
		  rcond_plus_one == 1.0 || xisnan (rcond))
		{
		  err = -2;
		  
		  if (sing_handler)
		    sing_handler (rcond);
		  else
		    (*current_liboctave_error_handler)
		      ("SparseComplexMatrix::solve matrix singular to machine precision, rcond = %g",
		       rcond);

		}
#endif

	      UMFPACK_ZNAME (report_info) (control, info);

	      UMFPACK_ZNAME (free_numeric) (&Numeric);
	    }
#else
	  (*current_liboctave_error_handler) ("UMFPACK not installed");
#endif
	}
      else if (typ != SparseType::Hermitian)
	(*current_liboctave_error_handler) ("incorrect matrix type");
    }
  
  return retval;
}

SparseComplexMatrix
SparseComplexMatrix::fsolve (SparseType &mattype, const SparseMatrix& b, 
			     octave_idx_type& err, double& rcond,
			     solve_singularity_handler sing_handler) const
{
  SparseComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  err = 0;

  if (nr == 0 || nc == 0 || nr != nc || nr != b.rows ())
    (*current_liboctave_error_handler)
      ("matrix dimension mismatch solution of linear equations");
  else
    {
      // Print spparms("spumoni") info if requested
      volatile int typ = mattype.type ();
      mattype.info ();

      if (typ == SparseType::Hermitian)
	{
#ifdef HAVE_CHOLMOD
	  cholmod_common Common;
	  cholmod_common *cm = &Common;

	  // Setup initial parameters
	  CHOLMOD_NAME(start) (cm);
	  cm->prefer_zomplex = false;

	  double spu = Voctave_sparse_controls.get_key ("spumoni");
	  if (spu == 0.)
	    {
	      cm->print = -1;
	      cm->print_function = NULL;
	    }
	  else
	    {
	      cm->print = (int)spu + 2;
	      cm->print_function =&SparseCholPrint;
	    }

	  cm->error_handler = &SparseCholError;
	  cm->complex_divide = CHOLMOD_NAME(divcomplex);
	  cm->hypotenuse = CHOLMOD_NAME(hypot);

#ifdef HAVE_METIS
	  // METIS 4.0.1 uses malloc and free, and will terminate MATLAB if
	  // it runs out of memory.  Use CHOLMOD's memory guard for METIS, 
	  // which mxMalloc's a huge block of memory (and then immediately 
	  // mxFree's it) before calling METIS
	  cm->metis_memory = 2.0;

#if defined(METIS_VERSION)
#if (METIS_VERSION >= METIS_VER(4,0,2))
	  // METIS 4.0.2 uses function pointers for malloc and free
	  METIS_malloc = cm->malloc_memory;
	  METIS_free   = cm->free_memory;
	  // Turn off METIS memory guard.  It is not needed, because mxMalloc
	  // will safely terminate the mexFunction and free any workspace
	  // without killing all of octave.
	  cm->metis_memory   = 0.0;
#endif
#endif
#endif

	  cm->final_ll = true;

	  cholmod_sparse Astore;
	  cholmod_sparse *A = &Astore;
	  double dummy;
	  A->nrow = nr;
	  A->ncol = nc;

	  A->p = cidx();
	  A->i = ridx();
	  A->nzmax = nonzero();
	  A->packed = true;
	  A->sorted = true;
	  A->nz = NULL;
#ifdef IDX_TYPE_LONG
	  A->itype = CHOLMOD_LONG;
#else
	  A->itype = CHOLMOD_INT;
#endif
	  A->dtype = CHOLMOD_DOUBLE;
	  A->stype = 1;
	  A->xtype = CHOLMOD_COMPLEX;

	  if (nr < 1)
	    A->x = &dummy;
	  else
	    A->x = data();

	  cholmod_sparse Bstore;
	  cholmod_sparse *B = &Bstore;
	  B->nrow = b.rows();
	  B->ncol = b.cols();
	  B->p = b.cidx();
	  B->i = b.ridx();
	  B->nzmax = b.nonzero();
	  B->packed = true;
	  B->sorted = true;
	  B->nz = NULL;
#ifdef IDX_TYPE_LONG
	  B->itype = CHOLMOD_LONG;
#else
	  B->itype = CHOLMOD_INT;
#endif
	  B->dtype = CHOLMOD_DOUBLE;
	  B->stype = 0;
	  B->xtype = CHOLMOD_REAL;

	  if (b.rows() < 1 || b.cols() < 1)
	    B->x = &dummy;
	  else
	    B->x = b.data();

	  cholmod_factor *L;
	  BEGIN_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;
	  L = CHOLMOD_NAME(analyze) (A, cm);
	  CHOLMOD_NAME(factorize) (A, L, cm);
	  rcond = CHOLMOD_NAME(rcond)(L, cm);
	  END_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;

	  if (rcond == 0.0)
	    {
	      // Either its indefinite or singular. Try UMFPACK
	      mattype.mark_as_unsymmetric ();
	      typ = SparseType::Full;
	    }
	  else
	    {
	      volatile double rcond_plus_one = rcond + 1.0;

	      if (rcond_plus_one == 1.0 || xisnan (rcond))
		{
		  err = -2;

		  if (sing_handler)
		    sing_handler (rcond);
		  else
		    (*current_liboctave_error_handler)
		      ("SparseMatrix::solve matrix singular to machine precision, rcond = %g",
		       rcond);
	      
#ifdef HAVE_LSSOLVE
		  return retval;
#endif
		}

	      cholmod_sparse *X;
	      BEGIN_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;
	      X = CHOLMOD_NAME(spsolve) (CHOLMOD_A, L, B, cm);
	      END_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;

	      retval = SparseComplexMatrix 
		(static_cast<octave_idx_type>(X->nrow), 
		 static_cast<octave_idx_type>(X->ncol),
		 static_cast<octave_idx_type>(X->nzmax));
	      for (octave_idx_type j = 0; 
		   j <= static_cast<octave_idx_type>(X->ncol); j++)
		retval.xcidx(j) = static_cast<octave_idx_type *>(X->p)[j];
	      for (octave_idx_type j = 0; 
		   j < static_cast<octave_idx_type>(X->nzmax); j++)
		{
		  retval.xridx(j) = static_cast<octave_idx_type *>(X->i)[j];
		  retval.xdata(j) = static_cast<Complex *>(X->x)[j];
		}

	      BEGIN_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;
	      CHOLMOD_NAME(free_sparse) (&X, cm);
	      CHOLMOD_NAME(free_factor) (&L, cm);
	      CHOLMOD_NAME(finish) (cm);
	      CHOLMOD_NAME(print_common) (" ", cm);
	      END_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;
	    }
#else
	  (*current_liboctave_warning_handler)
	    ("CHOLMOD not installed");

	  mattype.mark_as_unsymmetric ();
	  typ = SparseType::Full;
#endif
	}

      if (typ == SparseType::Full)
	{
#ifdef HAVE_UMFPACK
	  Matrix Control, Info;
	  void *Numeric = factorize (err, rcond, Control, Info, sing_handler);

	  if (err == 0)
	    {
	      octave_idx_type b_nr = b.rows ();
	      octave_idx_type b_nc = b.cols ();
	      int status = 0;
	      double *control = Control.fortran_vec ();
	      double *info = Info.fortran_vec ();
	      const octave_idx_type *Ap = cidx ();
	      const octave_idx_type *Ai = ridx ();
	      const Complex *Ax = data ();

#ifdef UMFPACK_SEPARATE_SPLIT
	      OCTAVE_LOCAL_BUFFER (double, Bx, b_nr);
	      OCTAVE_LOCAL_BUFFER (double, Bz, b_nr);
	      for (octave_idx_type i = 0; i < b_nr; i++)
		Bz[i] = 0.;
#else
	      OCTAVE_LOCAL_BUFFER (Complex, Bz, b_nr);
#endif

	      // Take a first guess that the number of non-zero terms
	      // will be as many as in b
	      octave_idx_type x_nz = b.nnz ();
	      octave_idx_type ii = 0;
	      retval = SparseComplexMatrix (b_nr, b_nc, x_nz);

	      OCTAVE_LOCAL_BUFFER (Complex, Xx, b_nr);
	      
	      retval.xcidx(0) = 0;
	      for (octave_idx_type j = 0; j < b_nc; j++)
		{

#ifdef UMFPACK_SEPARATE_SPLIT
		  for (octave_idx_type i = 0; i < b_nr; i++)
		    Bx[i] = b.elem (i, j);

		  status = UMFPACK_ZNAME (solve) (UMFPACK_A, Ap,
					     Ai, X_CAST (const double *, Ax),
					     NULL,
					     X_CAST (double *, Xx), NULL, 
					     Bx, Bz, Numeric, control, 
					     info);
#else
		  for (octave_idx_type i = 0; i < b_nr; i++)
		    Bz[i] = b.elem (i, j);

		  status = UMFPACK_ZNAME (solve) (UMFPACK_A, Ap, Ai, 
					     X_CAST (const double *, Ax),
					     NULL,
					     X_CAST (double *, Xx), NULL, 
					     X_CAST (double *, Bz), NULL,
					     Numeric, control, 
					     info);
#endif
		  if (status < 0)
		    {
		      (*current_liboctave_error_handler) 
			("SparseComplexMatrix::solve solve failed");

		      UMFPACK_ZNAME (report_status) (control, status);
		      
		      err = -1;

		      break;
		    }

		  for (octave_idx_type i = 0; i < b_nr; i++)
		    {
		      Complex tmp = Xx[i];
		      if (tmp != 0.0)
			{
			  if (ii == x_nz)
			    {
			      // Resize the sparse matrix
			      octave_idx_type sz = x_nz * (b_nc - j) / b_nc;
			      sz = (sz > 10 ? sz : 10) + x_nz;
			      retval.change_capacity (sz);
			      x_nz = sz;
			    }
			  retval.xdata(ii) = tmp;
			  retval.xridx(ii++) = i;
			}
		    }
		  retval.xcidx(j+1) = ii;
		}

	      retval.maybe_compress ();

#ifndef HAVE_LSSOLVE
	      rcond = Info (UMFPACK_RCOND);
	      volatile double rcond_plus_one = rcond + 1.0;

	      if (status == UMFPACK_WARNING_singular_matrix || 
		  rcond_plus_one == 1.0 || xisnan (rcond))
		{
		  err = -2;

		  if (sing_handler)
		    sing_handler (rcond);
		  else
		    (*current_liboctave_error_handler)
		      ("SparseComplexMatrix::solve matrix singular to machine precision, rcond = %g",
		       rcond);

		}
#endif

	      UMFPACK_ZNAME (report_info) (control, info);

	      UMFPACK_ZNAME (free_numeric) (&Numeric);
	    }
#else
	  (*current_liboctave_error_handler) ("UMFPACK not installed");
#endif
	}
      else if (typ != SparseType::Hermitian)
	(*current_liboctave_error_handler) ("incorrect matrix type");
    }
  
  return retval;
}

ComplexMatrix
SparseComplexMatrix::fsolve (SparseType &mattype, const ComplexMatrix& b, 
			     octave_idx_type& err, double& rcond,
			     solve_singularity_handler sing_handler) const
{
  ComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  err = 0;

  if (nr == 0 || nc == 0 || nr != nc || nr != b.rows ())
    (*current_liboctave_error_handler)
      ("matrix dimension mismatch solution of linear equations");
  else
    {
      // Print spparms("spumoni") info if requested
      volatile int typ = mattype.type ();
      mattype.info ();

      if (typ == SparseType::Hermitian)
	{
#ifdef HAVE_CHOLMOD
	  cholmod_common Common;
	  cholmod_common *cm = &Common;

	  // Setup initial parameters
	  CHOLMOD_NAME(start) (cm);
	  cm->prefer_zomplex = false;

	  double spu = Voctave_sparse_controls.get_key ("spumoni");
	  if (spu == 0.)
	    {
	      cm->print = -1;
	      cm->print_function = NULL;
	    }
	  else
	    {
	      cm->print = (int)spu + 2;
	      cm->print_function =&SparseCholPrint;
	    }

	  cm->error_handler = &SparseCholError;
	  cm->complex_divide = CHOLMOD_NAME(divcomplex);
	  cm->hypotenuse = CHOLMOD_NAME(hypot);

#ifdef HAVE_METIS
	  // METIS 4.0.1 uses malloc and free, and will terminate MATLAB if
	  // it runs out of memory.  Use CHOLMOD's memory guard for METIS, 
	  // which mxMalloc's a huge block of memory (and then immediately 
	  // mxFree's it) before calling METIS
	  cm->metis_memory = 2.0;

#if defined(METIS_VERSION)
#if (METIS_VERSION >= METIS_VER(4,0,2))
	  // METIS 4.0.2 uses function pointers for malloc and free
	  METIS_malloc = cm->malloc_memory;
	  METIS_free   = cm->free_memory;
	  // Turn off METIS memory guard.  It is not needed, because mxMalloc
	  // will safely terminate the mexFunction and free any workspace
	  // without killing all of octave.
	  cm->metis_memory   = 0.0;
#endif
#endif
#endif

	  cm->final_ll = true;

	  cholmod_sparse Astore;
	  cholmod_sparse *A = &Astore;
	  double dummy;
	  A->nrow = nr;
	  A->ncol = nc;

	  A->p = cidx();
	  A->i = ridx();
	  A->nzmax = nonzero();
	  A->packed = true;
	  A->sorted = true;
	  A->nz = NULL;
#ifdef IDX_TYPE_LONG
	  A->itype = CHOLMOD_LONG;
#else
	  A->itype = CHOLMOD_INT;
#endif
	  A->dtype = CHOLMOD_DOUBLE;
	  A->stype = 1;
	  A->xtype = CHOLMOD_COMPLEX;

	  if (nr < 1)
	    A->x = &dummy;
	  else
	    A->x = data();

	  cholmod_dense Bstore;
	  cholmod_dense *B = &Bstore;
	  B->nrow = b.rows();
	  B->ncol = b.cols();
	  B->d = B->nrow;
	  B->nzmax = B->nrow * B->ncol;
	  B->dtype = CHOLMOD_DOUBLE;
	  B->xtype = CHOLMOD_COMPLEX;
	  if (nc < 1 || b.cols() < 1)
	    B->x = &dummy;
	  else
	    // We won't alter it, honest :-)
	    B->x = const_cast<Complex *>(b.fortran_vec());

	  cholmod_factor *L;
	  BEGIN_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;
	  L = CHOLMOD_NAME(analyze) (A, cm);
	  CHOLMOD_NAME(factorize) (A, L, cm);
	  rcond = CHOLMOD_NAME(rcond)(L, cm);
	  END_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;

	  if (rcond == 0.0)
	    {
	      // Either its indefinite or singular. Try UMFPACK
	      mattype.mark_as_unsymmetric ();
	      typ = SparseType::Full;
	    }
	  else
	    {
	      volatile double rcond_plus_one = rcond + 1.0;

	      if (rcond_plus_one == 1.0 || xisnan (rcond))
		{
		  err = -2;

		  if (sing_handler)
		    sing_handler (rcond);
		  else
		    (*current_liboctave_error_handler)
		      ("SparseMatrix::solve matrix singular to machine precision, rcond = %g",
		       rcond);
	      
#ifdef HAVE_LSSOLVE
		  return retval;
#endif
		}

	      cholmod_dense *X;
	      BEGIN_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;
	      X = CHOLMOD_NAME(solve) (CHOLMOD_A, L, B, cm);
	      END_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;

	      retval.resize (b.rows (), b.cols());
	      for (octave_idx_type j = 0; j < b.cols(); j++)
		{
		  octave_idx_type jr = j * b.rows();
		  for (octave_idx_type i = 0; i < b.rows(); i++)
		    retval.xelem(i,j) = static_cast<Complex *>(X->x)[jr + i];
		}

	      BEGIN_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;
	      CHOLMOD_NAME(free_dense) (&X, cm);
	      CHOLMOD_NAME(free_factor) (&L, cm);
	      CHOLMOD_NAME(finish) (cm);
	      CHOLMOD_NAME(print_common) (" ", cm);
	      END_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;
	    }
#else
	  (*current_liboctave_warning_handler)
	    ("CHOLMOD not installed");

	  mattype.mark_as_unsymmetric ();
	  typ = SparseType::Full;
#endif
	}

      if (typ == SparseType::Full)
	{
#ifdef HAVE_UMFPACK
	  Matrix Control, Info;
	  void *Numeric = factorize (err, rcond, Control, Info, sing_handler);

	  if (err == 0)
	    {
	      octave_idx_type b_nr = b.rows ();
	      octave_idx_type b_nc = b.cols ();
	      int status = 0;
	      double *control = Control.fortran_vec ();
	      double *info = Info.fortran_vec ();
	      const octave_idx_type *Ap = cidx ();
	      const octave_idx_type *Ai = ridx ();
	      const Complex *Ax = data ();
	      const Complex *Bx = b.fortran_vec ();

	      retval.resize (b_nr, b_nc);
	      Complex *Xx = retval.fortran_vec ();
	      
	      for (octave_idx_type j = 0, iidx = 0; j < b_nc; j++, iidx += b_nr)
		{
		  status = 
		    UMFPACK_ZNAME (solve) (UMFPACK_A, Ap, Ai, 
				      X_CAST (const double *, Ax), 
				      NULL, X_CAST (double *, &Xx[iidx]), 
				      NULL, X_CAST (const double *, &Bx[iidx]), 
				      NULL, Numeric, control, info);
		  
		  if (status < 0)
		    {
		      (*current_liboctave_error_handler) 
			("SparseComplexMatrix::solve solve failed");

		      UMFPACK_ZNAME (report_status) (control, status);
		      
		      err = -1;

		      break;
		    }
		}

#ifndef HAVE_LSSOLVE
	      rcond = Info (UMFPACK_RCOND);
	      volatile double rcond_plus_one = rcond + 1.0;

	      if (status == UMFPACK_WARNING_singular_matrix || 
		  rcond_plus_one == 1.0 || xisnan (rcond))
		{
		  err = -2;

		  if (sing_handler)
		    sing_handler (rcond);
		  else
		    (*current_liboctave_error_handler)
		      ("SparseComplexMatrix::solve matrix singular to machine precision, rcond = %g",
		       rcond);

		}
#endif

	      UMFPACK_ZNAME (report_info) (control, info);

	      UMFPACK_ZNAME (free_numeric) (&Numeric);
	    }
#else
	  (*current_liboctave_error_handler) ("UMFPACK not installed");
#endif
	}
      else if (typ != SparseType::Hermitian)
	(*current_liboctave_error_handler) ("incorrect matrix type");
    }
  
  return retval;
}

SparseComplexMatrix
SparseComplexMatrix::fsolve (SparseType &mattype, const SparseComplexMatrix& b,
			     octave_idx_type& err, double& rcond,
			     solve_singularity_handler sing_handler) const
{
  SparseComplexMatrix retval;

  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  err = 0;

  if (nr == 0 || nc == 0 || nr != nc || nr != b.rows ())
    (*current_liboctave_error_handler)
      ("matrix dimension mismatch solution of linear equations");
  else
    {
      // Print spparms("spumoni") info if requested
      volatile int typ = mattype.type ();
      mattype.info ();

      if (typ == SparseType::Hermitian)
	{
#ifdef HAVE_CHOLMOD
	  cholmod_common Common;
	  cholmod_common *cm = &Common;

	  // Setup initial parameters
	  CHOLMOD_NAME(start) (cm);
	  cm->prefer_zomplex = false;

	  double spu = Voctave_sparse_controls.get_key ("spumoni");
	  if (spu == 0.)
	    {
	      cm->print = -1;
	      cm->print_function = NULL;
	    }
	  else
	    {
	      cm->print = (int)spu + 2;
	      cm->print_function =&SparseCholPrint;
	    }

	  cm->error_handler = &SparseCholError;
	  cm->complex_divide = CHOLMOD_NAME(divcomplex);
	  cm->hypotenuse = CHOLMOD_NAME(hypot);

#ifdef HAVE_METIS
	  // METIS 4.0.1 uses malloc and free, and will terminate MATLAB if
	  // it runs out of memory.  Use CHOLMOD's memory guard for METIS, 
	  // which mxMalloc's a huge block of memory (and then immediately 
	  // mxFree's it) before calling METIS
	  cm->metis_memory = 2.0;

#if defined(METIS_VERSION)
#if (METIS_VERSION >= METIS_VER(4,0,2))
	  // METIS 4.0.2 uses function pointers for malloc and free
	  METIS_malloc = cm->malloc_memory;
	  METIS_free   = cm->free_memory;
	  // Turn off METIS memory guard.  It is not needed, because mxMalloc
	  // will safely terminate the mexFunction and free any workspace
	  // without killing all of octave.
	  cm->metis_memory   = 0.0;
#endif
#endif
#endif

	  cm->final_ll = true;

	  cholmod_sparse Astore;
	  cholmod_sparse *A = &Astore;
	  double dummy;
	  A->nrow = nr;
	  A->ncol = nc;

	  A->p = cidx();
	  A->i = ridx();
	  A->nzmax = nonzero();
	  A->packed = true;
	  A->sorted = true;
	  A->nz = NULL;
#ifdef IDX_TYPE_LONG
	  A->itype = CHOLMOD_LONG;
#else
	  A->itype = CHOLMOD_INT;
#endif
	  A->dtype = CHOLMOD_DOUBLE;
	  A->stype = 1;
	  A->xtype = CHOLMOD_COMPLEX;

	  if (nr < 1)
	    A->x = &dummy;
	  else
	    A->x = data();

	  cholmod_sparse Bstore;
	  cholmod_sparse *B = &Bstore;
	  B->nrow = b.rows();
	  B->ncol = b.cols();
	  B->p = b.cidx();
	  B->i = b.ridx();
	  B->nzmax = b.nonzero();
	  B->packed = true;
	  B->sorted = true;
	  B->nz = NULL;
#ifdef IDX_TYPE_LONG
	  B->itype = CHOLMOD_LONG;
#else
	  B->itype = CHOLMOD_INT;
#endif
	  B->dtype = CHOLMOD_DOUBLE;
	  B->stype = 0;
	  B->xtype = CHOLMOD_COMPLEX;

	  if (b.rows() < 1 || b.cols() < 1)
	    B->x = &dummy;
	  else
	    B->x = b.data();

	  cholmod_factor *L;
	  BEGIN_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;
	  L = CHOLMOD_NAME(analyze) (A, cm);
	  CHOLMOD_NAME(factorize) (A, L, cm);
	  rcond = CHOLMOD_NAME(rcond)(L, cm);
	  END_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;

	  if (rcond == 0.0)
	    {
	      // Either its indefinite or singular. Try UMFPACK
	      mattype.mark_as_unsymmetric ();
	      typ = SparseType::Full;
	    }
	  else
	    {
	      volatile double rcond_plus_one = rcond + 1.0;

	      if (rcond_plus_one == 1.0 || xisnan (rcond))
		{
		  err = -2;

		  if (sing_handler)
		    sing_handler (rcond);
		  else
		    (*current_liboctave_error_handler)
		      ("SparseMatrix::solve matrix singular to machine precision, rcond = %g",
		       rcond);
	      
#ifdef HAVE_LSSOLVE
		  return retval;
#endif
		}

	      cholmod_sparse *X;
	      BEGIN_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;
	      X = CHOLMOD_NAME(spsolve) (CHOLMOD_A, L, B, cm);
	      END_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;

	      retval = SparseComplexMatrix 
		(static_cast<octave_idx_type>(X->nrow), 
		 static_cast<octave_idx_type>(X->ncol),
		 static_cast<octave_idx_type>(X->nzmax));
	      for (octave_idx_type j = 0; 
		   j <= static_cast<octave_idx_type>(X->ncol); j++)
		retval.xcidx(j) = static_cast<octave_idx_type *>(X->p)[j];
	      for (octave_idx_type j = 0; 
		   j < static_cast<octave_idx_type>(X->nzmax); j++)
		{
		  retval.xridx(j) = static_cast<octave_idx_type *>(X->i)[j];
		  retval.xdata(j) = static_cast<Complex *>(X->x)[j];
		}

	      BEGIN_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;
	      CHOLMOD_NAME(free_sparse) (&X, cm);
	      CHOLMOD_NAME(free_factor) (&L, cm);
	      CHOLMOD_NAME(finish) (cm);
	      CHOLMOD_NAME(print_common) (" ", cm);
	      END_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;
	    }
#else
	  (*current_liboctave_warning_handler)
	    ("CHOLMOD not installed");

	  mattype.mark_as_unsymmetric ();
	  typ = SparseType::Full;
#endif
	}

      if (typ == SparseType::Full)
	{
#ifdef HAVE_UMFPACK
	  Matrix Control, Info;
	  void *Numeric = factorize (err, rcond, Control, Info, sing_handler);

	  if (err == 0)
	    {
	      octave_idx_type b_nr = b.rows ();
	      octave_idx_type b_nc = b.cols ();
	      int status = 0;
	      double *control = Control.fortran_vec ();
	      double *info = Info.fortran_vec ();
	      const octave_idx_type *Ap = cidx ();
	      const octave_idx_type *Ai = ridx ();
	      const Complex *Ax = data ();

	      OCTAVE_LOCAL_BUFFER (Complex, Bx, b_nr);

	      // Take a first guess that the number of non-zero terms
	      // will be as many as in b
	      octave_idx_type x_nz = b.nnz ();
	      octave_idx_type ii = 0;
	      retval = SparseComplexMatrix (b_nr, b_nc, x_nz);

	      OCTAVE_LOCAL_BUFFER (Complex, Xx, b_nr);
	      
	      retval.xcidx(0) = 0;
	      for (octave_idx_type j = 0; j < b_nc; j++)
		{
		  for (octave_idx_type i = 0; i < b_nr; i++)
		    Bx[i] = b (i,j);

		  status = UMFPACK_ZNAME (solve) (UMFPACK_A, Ap,
					     Ai, X_CAST (const double *, Ax), 
					     NULL, X_CAST (double *, Xx), 
					     NULL, X_CAST (double *, Bx), 
					     NULL, Numeric, control, info);
		  
		  if (status < 0)
		    {
		      (*current_liboctave_error_handler) 
			("SparseComplexMatrix::solve solve failed");

		      UMFPACK_ZNAME (report_status) (control, status);
		  
		      err = -1;

		      break;
		    }

		  for (octave_idx_type i = 0; i < b_nr; i++)
		    {
		      Complex tmp = Xx[i];
		      if (tmp != 0.0)
			{
			  if (ii == x_nz)
			    {
			      // Resize the sparse matrix
			      octave_idx_type sz = x_nz * (b_nc - j) / b_nc;
			      sz = (sz > 10 ? sz : 10) + x_nz;
			      retval.change_capacity (sz);
			      x_nz = sz;
			    }
			  retval.xdata(ii) = tmp;
			  retval.xridx(ii++) = i;
			}
		    }
		  retval.xcidx(j+1) = ii;
		}

	      retval.maybe_compress ();

#ifndef HAVE_LSSOLVE
	      rcond = Info (UMFPACK_RCOND);
	      volatile double rcond_plus_one = rcond + 1.0;

	      if (status == UMFPACK_WARNING_singular_matrix || 
		  rcond_plus_one == 1.0 || xisnan (rcond))
		{
		  err = -2;

		  if (sing_handler)
		    sing_handler (rcond);
		  else
		    (*current_liboctave_error_handler)
		      ("SparseComplexMatrix::solve matrix singular to machine precision, rcond = %g",
		       rcond);

		}
#endif

	      UMFPACK_ZNAME (report_info) (control, info);

	      UMFPACK_ZNAME (free_numeric) (&Numeric);
	    }
#else
	  (*current_liboctave_error_handler) ("UMFPACK not installed");
#endif
	}
      else if (typ != SparseType::Hermitian)
	(*current_liboctave_error_handler) ("incorrect matrix type");
    }
  
  return retval;
}

ComplexMatrix
SparseComplexMatrix::solve (SparseType &mattype, const Matrix& b) const
{
  octave_idx_type info;
  double rcond;
  return solve (mattype, b, info, rcond, 0);
}

ComplexMatrix
SparseComplexMatrix::solve (SparseType &mattype, const Matrix& b, 
			    octave_idx_type& info) const
{
  double rcond;
  return solve (mattype, b, info, rcond, 0);
}

ComplexMatrix
SparseComplexMatrix::solve (SparseType &mattype, const Matrix& b, octave_idx_type& info, 
			    double& rcond) const
{
  return solve (mattype, b, info, rcond, 0);
}

ComplexMatrix
SparseComplexMatrix::solve (SparseType &mattype, const Matrix& b, octave_idx_type& err, 
			    double& rcond, 
			    solve_singularity_handler sing_handler) const
{
  int typ = mattype.type (false);

  if (typ == SparseType::Unknown)
    typ = mattype.type (*this);

  if (typ == SparseType::Diagonal || typ == SparseType::Permuted_Diagonal)
    return dsolve (mattype, b, err, rcond, sing_handler);
  else if (typ == SparseType::Upper || typ == SparseType::Permuted_Upper)
    return utsolve (mattype, b, err, rcond, sing_handler);
  else if (typ == SparseType::Lower || typ == SparseType::Permuted_Lower)
    return ltsolve (mattype, b, err, rcond, sing_handler);
  else if (typ == SparseType::Banded || typ == SparseType::Banded_Hermitian)
    return bsolve (mattype, b, err, rcond, sing_handler);
  else if (typ == SparseType::Tridiagonal || 
	   typ == SparseType::Tridiagonal_Hermitian)
    return trisolve (mattype, b, err, rcond, sing_handler);
  else if (typ == SparseType::Full || typ == SparseType::Hermitian)
    return fsolve (mattype, b, err, rcond, sing_handler);
  else
    {
      (*current_liboctave_error_handler) 
	("matrix dimension mismatch solution of linear equations");
      return ComplexMatrix ();
    }
}

SparseComplexMatrix
SparseComplexMatrix::solve (SparseType &mattype, const SparseMatrix& b) const
{
  octave_idx_type info;
  double rcond;
  return solve (mattype, b, info, rcond, 0);
}

SparseComplexMatrix
SparseComplexMatrix::solve (SparseType &mattype, const SparseMatrix& b, 
		     octave_idx_type& info) const
{
  double rcond;
  return solve (mattype, b, info, rcond, 0);
}

SparseComplexMatrix
SparseComplexMatrix::solve (SparseType &mattype, const SparseMatrix& b,
		     octave_idx_type& info, double& rcond) const
{
  return solve (mattype, b, info, rcond, 0);
}

SparseComplexMatrix
SparseComplexMatrix::solve (SparseType &mattype, const SparseMatrix& b, 
			    octave_idx_type& err, double& rcond,
			    solve_singularity_handler sing_handler) const
{
  int typ = mattype.type (false);

  if (typ == SparseType::Unknown)
    typ = mattype.type (*this);

  if (typ == SparseType::Diagonal || typ == SparseType::Permuted_Diagonal)
    return dsolve (mattype, b, err, rcond, sing_handler);
  else if (typ == SparseType::Upper || typ == SparseType::Permuted_Upper)
    return utsolve (mattype, b, err, rcond, sing_handler);
  else if (typ == SparseType::Lower || typ == SparseType::Permuted_Lower)
    return ltsolve (mattype, b, err, rcond, sing_handler);
  else if (typ == SparseType::Banded || typ == SparseType::Banded_Hermitian)
    return bsolve (mattype, b, err, rcond, sing_handler);
  else if (typ == SparseType::Tridiagonal || 
	   typ == SparseType::Tridiagonal_Hermitian)
    return trisolve (mattype, b, err, rcond, sing_handler);
  else if (typ == SparseType::Full || typ == SparseType::Hermitian)
    return fsolve (mattype, b, err, rcond, sing_handler);
  else
    {
      (*current_liboctave_error_handler) 
	("matrix dimension mismatch solution of linear equations");
      return SparseComplexMatrix ();
    }
}

ComplexMatrix
SparseComplexMatrix::solve (SparseType &mattype, const ComplexMatrix& b) const
{
  octave_idx_type info;
  double rcond;
  return solve (mattype, b, info, rcond, 0);
}

ComplexMatrix
SparseComplexMatrix::solve (SparseType &mattype, const ComplexMatrix& b, 
			    octave_idx_type& info) const
{
  double rcond;
  return solve (mattype, b, info, rcond, 0);
}

ComplexMatrix
SparseComplexMatrix::solve (SparseType &mattype, const ComplexMatrix& b, 
		     octave_idx_type& info, double& rcond) const
{
  return solve (mattype, b, info, rcond, 0);
}

ComplexMatrix
SparseComplexMatrix::solve (SparseType &mattype, const ComplexMatrix& b, 
		     octave_idx_type& err, double& rcond, 
		     solve_singularity_handler sing_handler) const
{
  int typ = mattype.type (false);

  if (typ == SparseType::Unknown)
    typ = mattype.type (*this);

  if (typ == SparseType::Diagonal || typ == SparseType::Permuted_Diagonal)
    return dsolve (mattype, b, err, rcond, sing_handler);
  else if (typ == SparseType::Upper || typ == SparseType::Permuted_Upper)
    return utsolve (mattype, b, err, rcond, sing_handler);
  else if (typ == SparseType::Lower || typ == SparseType::Permuted_Lower)
    return ltsolve (mattype, b, err, rcond, sing_handler);
  else if (typ == SparseType::Banded || typ == SparseType::Banded_Hermitian)
    return bsolve (mattype, b, err, rcond, sing_handler);
  else if (typ == SparseType::Tridiagonal || 
	   typ == SparseType::Tridiagonal_Hermitian)
    return trisolve (mattype, b, err, rcond, sing_handler);
  else if (typ == SparseType::Full || typ == SparseType::Hermitian)
    return fsolve (mattype, b, err, rcond, sing_handler);
  else
    {
      (*current_liboctave_error_handler) 
	("matrix dimension mismatch solution of linear equations");
      return ComplexMatrix ();
    }
}

SparseComplexMatrix
SparseComplexMatrix::solve (SparseType &mattype, 
			    const SparseComplexMatrix& b) const
{
  octave_idx_type info;
  double rcond;
  return solve (mattype, b, info, rcond, 0);
}

SparseComplexMatrix
SparseComplexMatrix::solve (SparseType &mattype, const SparseComplexMatrix& b, 
		     octave_idx_type& info) const
{
  double rcond;
  return solve (mattype, b, info, rcond, 0);
}

SparseComplexMatrix
SparseComplexMatrix::solve (SparseType &mattype, const SparseComplexMatrix& b,
		     octave_idx_type& info, double& rcond) const
{
  return solve (mattype, b, info, rcond, 0);
}

SparseComplexMatrix
SparseComplexMatrix::solve (SparseType &mattype, const SparseComplexMatrix& b, 
			    octave_idx_type& err, double& rcond,
			    solve_singularity_handler sing_handler) const
{
  int typ = mattype.type (false);

  if (typ == SparseType::Unknown)
    typ = mattype.type (*this);

  if (typ == SparseType::Diagonal || typ == SparseType::Permuted_Diagonal)
    return dsolve (mattype, b, err, rcond, sing_handler);
  else if (typ == SparseType::Upper || typ == SparseType::Permuted_Upper)
    return utsolve (mattype, b, err, rcond, sing_handler);
  else if (typ == SparseType::Lower || typ == SparseType::Permuted_Lower)
    return ltsolve (mattype, b, err, rcond, sing_handler);
  else if (typ == SparseType::Banded || typ == SparseType::Banded_Hermitian)
    return bsolve (mattype, b, err, rcond, sing_handler);
  else if (typ == SparseType::Tridiagonal || 
	   typ == SparseType::Tridiagonal_Hermitian)
    return trisolve (mattype, b, err, rcond, sing_handler);
  else if (typ == SparseType::Full || typ == SparseType::Hermitian)
    return fsolve (mattype, b, err, rcond, sing_handler);
  else
    {
      (*current_liboctave_error_handler) 
	("matrix dimension mismatch solution of linear equations");
      return SparseComplexMatrix ();
    }
}

ComplexColumnVector
SparseComplexMatrix::solve (SparseType &mattype, const ColumnVector& b) const
{
  octave_idx_type info; double rcond;
  return solve (mattype, b, info, rcond);
}

ComplexColumnVector
SparseComplexMatrix::solve (SparseType &mattype, const ColumnVector& b, 
			    octave_idx_type& info) const
{
  double rcond;
  return solve (mattype, b, info, rcond);
}

ComplexColumnVector
SparseComplexMatrix::solve (SparseType &mattype, const ColumnVector& b, 
			    octave_idx_type& info, double& rcond) const
{
  return solve (mattype, b, info, rcond, 0);
}

ComplexColumnVector
SparseComplexMatrix::solve (SparseType &mattype, const ColumnVector& b, 
			    octave_idx_type& info, double& rcond,
			    solve_singularity_handler sing_handler) const
{
  Matrix tmp (b);
  return solve (mattype, tmp, info, rcond, sing_handler).column (static_cast<octave_idx_type> (0));
}

ComplexColumnVector
SparseComplexMatrix::solve (SparseType &mattype, 
			    const ComplexColumnVector& b) const
{
  octave_idx_type info;
  double rcond;
  return solve (mattype, b, info, rcond, 0);
}

ComplexColumnVector
SparseComplexMatrix::solve (SparseType &mattype, const ComplexColumnVector& b,
			    octave_idx_type& info) const
{
  double rcond;
  return solve (mattype, b, info, rcond, 0);
}

ComplexColumnVector
SparseComplexMatrix::solve (SparseType &mattype, const ComplexColumnVector& b,
			    octave_idx_type& info, double& rcond) const
{
  return solve (mattype, b, info, rcond, 0);
}

ComplexColumnVector
SparseComplexMatrix::solve (SparseType &mattype, const ComplexColumnVector& b,
			    octave_idx_type& info, double& rcond,
			    solve_singularity_handler sing_handler) const
{
  ComplexMatrix tmp (b);
  return solve (mattype, tmp, info, rcond, sing_handler).column (static_cast<octave_idx_type> (0));
}

ComplexMatrix
SparseComplexMatrix::solve (const Matrix& b) const
{
  octave_idx_type info;
  double rcond;
  return solve (b, info, rcond, 0);
}

ComplexMatrix
SparseComplexMatrix::solve (const Matrix& b, octave_idx_type& info) const
{
  double rcond;
  return solve (b, info, rcond, 0);
}

ComplexMatrix
SparseComplexMatrix::solve (const Matrix& b, octave_idx_type& info, 
		     double& rcond) const
{
  return solve (b, info, rcond, 0);
}

ComplexMatrix
SparseComplexMatrix::solve (const Matrix& b, octave_idx_type& err, 
			    double& rcond, 
			    solve_singularity_handler sing_handler) const
{
  SparseType mattype (*this);
  return solve (mattype, b, err, rcond, sing_handler);
}

SparseComplexMatrix
SparseComplexMatrix::solve (const SparseMatrix& b) const
{
  octave_idx_type info;
  double rcond;
  return solve (b, info, rcond, 0);
}

SparseComplexMatrix
SparseComplexMatrix::solve (const SparseMatrix& b, 
		     octave_idx_type& info) const
{
  double rcond;
  return solve (b, info, rcond, 0);
}

SparseComplexMatrix
SparseComplexMatrix::solve (const SparseMatrix& b,
		     octave_idx_type& info, double& rcond) const
{
  return solve (b, info, rcond, 0);
}

SparseComplexMatrix
SparseComplexMatrix::solve (const SparseMatrix& b, 
		     octave_idx_type& err, double& rcond,
		     solve_singularity_handler sing_handler) const
{
  SparseType mattype (*this);
  return solve (mattype, b, err, rcond, sing_handler);
}

ComplexMatrix
SparseComplexMatrix::solve (const ComplexMatrix& b, 
			    octave_idx_type& info) const
{
  double rcond;
  return solve (b, info, rcond, 0);
}

ComplexMatrix
SparseComplexMatrix::solve (const ComplexMatrix& b, 
		     octave_idx_type& info, double& rcond) const
{
  return solve (b, info, rcond, 0);
}

ComplexMatrix
SparseComplexMatrix::solve (const ComplexMatrix& b, 
		     octave_idx_type& err, double& rcond, 
		     solve_singularity_handler sing_handler) const
{
  SparseType mattype (*this);
  return solve (mattype, b, err, rcond, sing_handler);
}

SparseComplexMatrix
SparseComplexMatrix::solve (const SparseComplexMatrix& b) const
{
  octave_idx_type info;
  double rcond;
  return solve (b, info, rcond, 0);
}

SparseComplexMatrix
SparseComplexMatrix::solve (const SparseComplexMatrix& b, 
		     octave_idx_type& info) const
{
  double rcond;
  return solve (b, info, rcond, 0);
}

SparseComplexMatrix
SparseComplexMatrix::solve (const SparseComplexMatrix& b,
		     octave_idx_type& info, double& rcond) const
{
  return solve (b, info, rcond, 0);
}

SparseComplexMatrix
SparseComplexMatrix::solve (const SparseComplexMatrix& b, 
		     octave_idx_type& err, double& rcond,
		     solve_singularity_handler sing_handler) const
{
  SparseType mattype (*this);
  return solve (mattype, b, err, rcond, sing_handler);
}

ComplexColumnVector
SparseComplexMatrix::solve (const ColumnVector& b) const
{
  octave_idx_type info; double rcond;
  return solve (b, info, rcond);
}

ComplexColumnVector
SparseComplexMatrix::solve (const ColumnVector& b, octave_idx_type& info) const
{
  double rcond;
  return solve (b, info, rcond);
}

ComplexColumnVector
SparseComplexMatrix::solve (const ColumnVector& b, octave_idx_type& info, 
			    double& rcond) const
{
  return solve (b, info, rcond, 0);
}

ComplexColumnVector
SparseComplexMatrix::solve (const ColumnVector& b, octave_idx_type& info, double& rcond,
			    solve_singularity_handler sing_handler) const
{
  Matrix tmp (b);
  return solve (tmp, info, rcond, sing_handler).column (static_cast<octave_idx_type> (0));
}

ComplexColumnVector
SparseComplexMatrix::solve (const ComplexColumnVector& b) const
{
  octave_idx_type info;
  double rcond;
  return solve (b, info, rcond, 0);
}

ComplexColumnVector
SparseComplexMatrix::solve (const ComplexColumnVector& b, octave_idx_type& info) const
{
  double rcond;
  return solve (b, info, rcond, 0);
}

ComplexColumnVector
SparseComplexMatrix::solve (const ComplexColumnVector& b, octave_idx_type& info, 
		     double& rcond) const
{
  return solve (b, info, rcond, 0);
}

ComplexColumnVector
SparseComplexMatrix::solve (const ComplexColumnVector& b, octave_idx_type& info, 
			    double& rcond,
			    solve_singularity_handler sing_handler) const
{
  ComplexMatrix tmp (b);
  return solve (tmp, info, rcond, sing_handler).column (static_cast<octave_idx_type> (0));
}

ComplexMatrix
SparseComplexMatrix::lssolve (const Matrix& b) const
{
  octave_idx_type info;
  octave_idx_type rank;
  return lssolve (b, info, rank);
}

ComplexMatrix
SparseComplexMatrix::lssolve (const Matrix& b, octave_idx_type& info) const
{
  octave_idx_type rank;
  return lssolve (b, info, rank);
}

ComplexMatrix
SparseComplexMatrix::lssolve (const Matrix& b, octave_idx_type& info, octave_idx_type& rank) const
{
  info = -1;
  (*current_liboctave_error_handler) 
    ("SparseComplexMatrix::lssolve not implemented yet");
  return ComplexMatrix ();
}

SparseComplexMatrix
SparseComplexMatrix::lssolve (const SparseMatrix& b) const
{
  octave_idx_type info;
  octave_idx_type rank;
  return lssolve (b, info, rank);
}

SparseComplexMatrix
SparseComplexMatrix::lssolve (const SparseMatrix& b, octave_idx_type& info) const
{
  octave_idx_type rank;
  return lssolve (b, info, rank);
}

SparseComplexMatrix
SparseComplexMatrix::lssolve (const SparseMatrix& b, octave_idx_type& info, 
			      octave_idx_type& rank) const
{
  info = -1;
  (*current_liboctave_error_handler) 
    ("SparseComplexMatrix::lssolve not implemented yet");
  return SparseComplexMatrix ();
}

ComplexMatrix
SparseComplexMatrix::lssolve (const ComplexMatrix& b) const
{
  octave_idx_type info;
  octave_idx_type rank;
  return lssolve (b, info, rank);
}

ComplexMatrix
SparseComplexMatrix::lssolve (const ComplexMatrix& b, octave_idx_type& info) const
{
  octave_idx_type rank;
  return lssolve (b, info, rank);
}

ComplexMatrix
SparseComplexMatrix::lssolve (const ComplexMatrix& b, octave_idx_type& info, 
			      octave_idx_type& rank) const
{
  info = -1;
  (*current_liboctave_error_handler) 
    ("SparseComplexMatrix::lssolve not implemented yet");
  return ComplexMatrix ();
}

SparseComplexMatrix
SparseComplexMatrix::lssolve (const SparseComplexMatrix& b) const
{
  octave_idx_type info;
  octave_idx_type rank;
  return lssolve (b, info, rank);
}

SparseComplexMatrix
SparseComplexMatrix::lssolve (const SparseComplexMatrix& b, octave_idx_type& info) const
{
  octave_idx_type rank;
  return lssolve (b, info, rank);
}

SparseComplexMatrix
SparseComplexMatrix::lssolve (const SparseComplexMatrix& b, octave_idx_type& info, 
			      octave_idx_type& rank) const
{
  info = -1;
  (*current_liboctave_error_handler) 
    ("SparseComplexMatrix::lssolve not implemented yet");
  return SparseComplexMatrix ();
}

ComplexColumnVector
SparseComplexMatrix::lssolve (const ColumnVector& b) const
{
  octave_idx_type info;
  octave_idx_type rank;
  return lssolve (b, info, rank);
}

ComplexColumnVector
SparseComplexMatrix::lssolve (const ColumnVector& b, octave_idx_type& info) const
{
  octave_idx_type rank;
  return lssolve (b, info, rank);
}

ComplexColumnVector
SparseComplexMatrix::lssolve (const ColumnVector& b, octave_idx_type& info, octave_idx_type& rank) const
{
  info = -1;
  (*current_liboctave_error_handler) 
    ("SparseComplexMatrix::lssolve not implemented yet");
  return ComplexColumnVector ();
}

ComplexColumnVector
SparseComplexMatrix::lssolve (const ComplexColumnVector& b) const
{
  octave_idx_type info;
  octave_idx_type rank;
  return lssolve (b, info, rank);
}

ComplexColumnVector
SparseComplexMatrix::lssolve (const ComplexColumnVector& b, octave_idx_type& info) const
{
  octave_idx_type rank;
  return lssolve (b, info, rank);
}

ComplexColumnVector
SparseComplexMatrix::lssolve (const ComplexColumnVector& b, octave_idx_type& info,
			octave_idx_type& rank) const
{
  info = -1;
  (*current_liboctave_error_handler) 
    ("SparseComplexMatrix::lssolve not implemented yet");
  return ComplexColumnVector ();
}

// unary operations
SparseBoolMatrix
SparseComplexMatrix::operator ! (void) const
{
  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  octave_idx_type nz1 = nnz ();
  octave_idx_type nz2 = nr*nc - nz1;
   
  SparseBoolMatrix r (nr, nc, nz2);
   
  octave_idx_type ii = 0;
  octave_idx_type jj = 0;
  r.cidx (0) = 0;
  for (octave_idx_type i = 0; i < nc; i++)
    {
      for (octave_idx_type j = 0; j < nr; j++)
	{
	  if (jj < cidx(i+1) && ridx(jj) == j)
	    jj++;
	  else
	    {
	      r.data(ii) = true;
	      r.ridx(ii++) = j;
	    }
	}
      r.cidx (i+1) = ii;
    }

  return r;
}

SparseComplexMatrix 
SparseComplexMatrix::squeeze (void) const
{ 
  return MSparse<Complex>::squeeze (); 
}

SparseComplexMatrix
SparseComplexMatrix::index (idx_vector& i, int resize_ok) const 
{ 
  return MSparse<Complex>::index (i, resize_ok); 
}

SparseComplexMatrix
SparseComplexMatrix::index (idx_vector& i, idx_vector& j, int resize_ok) const 
{ 
  return MSparse<Complex>::index (i, j, resize_ok); 
}
  
SparseComplexMatrix
SparseComplexMatrix::index (Array<idx_vector>& ra_idx, int resize_ok) const 
{ 
  return MSparse<Complex>::index (ra_idx, resize_ok); 
}
SparseComplexMatrix
SparseComplexMatrix::reshape (const dim_vector& new_dims) const
{
  return MSparse<Complex>::reshape (new_dims);
}

SparseComplexMatrix
SparseComplexMatrix::permute (const Array<octave_idx_type>& vec, bool inv) const
{
  return MSparse<Complex>::permute (vec, inv);
}

SparseComplexMatrix
SparseComplexMatrix::ipermute (const Array<octave_idx_type>& vec) const
{
  return MSparse<Complex>::ipermute (vec);
}

// other operations

SparseComplexMatrix
SparseComplexMatrix::map (c_c_Mapper f) const
{
  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  octave_idx_type nz = nnz ();
  bool f_zero = (f(0.0) == 0.0);

  // Count number of non-zero elements
  octave_idx_type nel = (f_zero ? 0 : nr*nc - nz);
  for (octave_idx_type i = 0; i < nz; i++)
    if (f (data(i)) != 0.0)
      nel++;

  SparseComplexMatrix retval (nr, nc, nel);

  if (f_zero)
    {
      octave_idx_type ii = 0;
      for (octave_idx_type j = 0; j < nc; j++)
	{
	  for (octave_idx_type i = 0; i < nr; i++)
	    {
	      Complex tmp = f (elem (i, j));
	      if (tmp != 0.0)
		{
		  retval.data(ii) = tmp;
		  retval.ridx(ii++) = i;
		}
	    }
	  retval.cidx(j+1) = ii;
	}
    }
  else
    {
      octave_idx_type ii = 0;
      for (octave_idx_type j = 0; j < nc; j++)
	{
	  for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
	    {
	      retval.data(ii) = f (elem(i));
	      retval.ridx(ii++) = ridx(i);
	    }
	  retval.cidx(j+1) = ii;
	}
    }

  return retval;
}

SparseMatrix
SparseComplexMatrix::map (d_c_Mapper f) const
{
  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  octave_idx_type nz = nnz ();
  bool f_zero = (f(0.0) == 0.0);

  // Count number of non-zero elements
  octave_idx_type nel = (f_zero ? 0 : nr*nc - nz);
  for (octave_idx_type i = 0; i < nz; i++)
    if (f (data(i)) != 0.0)
      nel++;

  SparseMatrix retval (nr, nc, nel);

  if (f_zero)
    {
      octave_idx_type ii = 0;
      for (octave_idx_type j = 0; j < nc; j++)
	{
	  for (octave_idx_type i = 0; i < nr; i++)
	    {
	      double tmp = f (elem (i, j));
	      if (tmp != 0.0)
		{
		  retval.data(ii) = tmp;
		  retval.ridx(ii++) = i;
		}
	    }
	  retval.cidx(j+1) = ii;
	}
    }
  else
    {
      octave_idx_type ii = 0;
      for (octave_idx_type j = 0; j < nc; j++)
	{
	  for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
	    {
	      retval.data(ii) = f (elem(i));
	      retval.ridx(ii++) = ridx(i);
	    }
	  retval.cidx(j+1) = ii;
	}
    }

  return retval;
}

SparseBoolMatrix
SparseComplexMatrix::map (b_c_Mapper f) const
{
  octave_idx_type nr = rows ();
  octave_idx_type nc = cols ();
  octave_idx_type nz = nnz ();
  bool f_zero = f(0.0);

  // Count number of non-zero elements
  octave_idx_type nel = (f_zero ? 0 : nr*nc - nz);
  for (octave_idx_type i = 0; i < nz; i++)
    if (f (data(i)) != 0.0)
      nel++;

  SparseBoolMatrix retval (nr, nc, nel);

  if (f_zero)
    {
      octave_idx_type ii = 0;
      for (octave_idx_type j = 0; j < nc; j++)
	{
	  for (octave_idx_type i = 0; i < nr; i++)
	    {
	      bool tmp = f (elem (i, j));
	      if (tmp)
		{
		  retval.data(ii) = tmp;
		  retval.ridx(ii++) = i;
		}
	    }
	  retval.cidx(j+1) = ii;
	}
    }
  else
    {
      octave_idx_type ii = 0;
      for (octave_idx_type j = 0; j < nc; j++)
	{
	  for (octave_idx_type i = cidx(j); i < cidx(j+1); i++)
	    {
	      retval.data(ii) = f (elem(i));
	      retval.ridx(ii++) = ridx(i);
	    }
	  retval.cidx(j+1) = ii;
	}
    }

  return retval;
}

SparseComplexMatrix&
SparseComplexMatrix::apply (c_c_Mapper f)
{
  *this = map (f);
  return *this;
}

bool
SparseComplexMatrix::any_element_is_inf_or_nan (void) const
{
  octave_idx_type nel = nnz ();

  for (octave_idx_type i = 0; i < nel; i++)
    {
      Complex val = data (i);
      if (xisinf (val) || xisnan (val))
	return true;
    }

  return false;
}

// Return true if no elements have imaginary components.

bool
SparseComplexMatrix::all_elements_are_real (void) const
{
  octave_idx_type nel = nnz ();

  for (octave_idx_type i = 0; i < nel; i++)
    {
      double ip = std::imag (data (i));
      
      if (ip != 0.0 || lo_ieee_signbit (ip))
	return false;
    }

  return true;
}

// Return nonzero if any element of CM has a non-integer real or
// imaginary part.  Also extract the largest and smallest (real or
// imaginary) values and return them in MAX_VAL and MIN_VAL. 

bool
SparseComplexMatrix::all_integers (double& max_val, double& min_val) const
{
  octave_idx_type nel = nnz ();

  if (nel == 0)
    return false;

  max_val = std::real(data (0));
  min_val = std::real(data (0));

  for (octave_idx_type i = 0; i < nel; i++)
    {
	Complex val = data (i);

	double r_val = std::real (val);
	double i_val = std::imag (val);

	if (r_val > max_val)
	  max_val = r_val;

	if (i_val > max_val)
	  max_val = i_val;

	if (r_val < min_val)
	  min_val = r_val;

	if (i_val < min_val)
	  min_val = i_val;

	if (D_NINT (r_val) != r_val || D_NINT (i_val) != i_val)
	  return false;
    }

  return true;
}

bool
SparseComplexMatrix::too_large_for_float (void) const
{
  octave_idx_type nel = nnz ();

  for (octave_idx_type i = 0; i < nel; i++)
    {
	Complex val = data (i);

	double r_val = std::real (val);
	double i_val = std::imag (val);

	if (r_val > FLT_MAX
	    || i_val > FLT_MAX
	    || r_val < FLT_MIN
	    || i_val < FLT_MIN)
	  return true;
    }

  return false;
}

// XXX FIXME XXX Do these really belong here?  Maybe they should be
// in a base class?

SparseBoolMatrix
SparseComplexMatrix::all (int dim) const
{
  SPARSE_ALL_OP (dim);
}

SparseBoolMatrix
SparseComplexMatrix::any (int dim) const
{
  SPARSE_ANY_OP (dim);
}

SparseComplexMatrix
SparseComplexMatrix::cumprod (int dim) const
{
  SPARSE_CUMPROD (SparseComplexMatrix, Complex, cumprod);
}

SparseComplexMatrix
SparseComplexMatrix::cumsum (int dim) const
{
  SPARSE_CUMSUM (SparseComplexMatrix, Complex, cumsum);
}

SparseComplexMatrix
SparseComplexMatrix::prod (int dim) const
{
  SPARSE_REDUCTION_OP (SparseComplexMatrix, Complex, *=, 1.0, 1.0);
}

SparseComplexMatrix
SparseComplexMatrix::sum (int dim) const
{
  SPARSE_REDUCTION_OP (SparseComplexMatrix, Complex, +=, 0.0, 0.0);
}

SparseComplexMatrix
SparseComplexMatrix::sumsq (int dim) const
{
#define ROW_EXPR \
  Complex d = elem (i, j); \
  tmp [i] += d * conj (d)

#define COL_EXPR \
  Complex d = elem (i, j); \
  tmp [j] += d * conj (d)

  SPARSE_BASE_REDUCTION_OP (SparseComplexMatrix, Complex, ROW_EXPR, 
			    COL_EXPR, 0.0, 0.0);

#undef ROW_EXPR
#undef COL_EXPR
}

SparseMatrix SparseComplexMatrix::abs (void) const
{
  octave_idx_type nz = nnz ();
  octave_idx_type nc = cols ();

  SparseMatrix retval (rows(), nc, nz);

  for (octave_idx_type i = 0; i < nc + 1; i++)
    retval.cidx (i) = cidx (i);

  for (octave_idx_type i = 0; i < nz; i++)
    {
      retval.data (i) = std::abs (data (i));
      retval.ridx (i) = ridx (i);
    }

  return retval;
}

SparseComplexMatrix
SparseComplexMatrix::diag (octave_idx_type k) const
{
  octave_idx_type nnr = rows ();
  octave_idx_type nnc = cols ();

  if (k > 0)
    nnc -= k;
  else if (k < 0)
    nnr += k;

  SparseComplexMatrix d;

  if (nnr > 0 && nnc > 0)
    {
      octave_idx_type ndiag = (nnr < nnc) ? nnr : nnc;

      // Count the number of non-zero elements
      octave_idx_type nel = 0;
      if (k > 0)
	{
	  for (octave_idx_type i = 0; i < ndiag; i++)
	    if (elem (i, i+k) != 0.)
	      nel++;
	}
      else if ( k < 0)
	{
	  for (octave_idx_type i = 0; i < ndiag; i++)
	    if (elem (i-k, i) != 0.)
	      nel++;
	}
      else
	{
	  for (octave_idx_type i = 0; i < ndiag; i++)
	    if (elem (i, i) != 0.)
	      nel++;
	}
      
      d = SparseComplexMatrix (ndiag, 1, nel);
      d.xcidx (0) = 0;
      d.xcidx (1) = nel;

      octave_idx_type ii = 0;
      if (k > 0)
	{
	  for (octave_idx_type i = 0; i < ndiag; i++)
	    {
	      Complex tmp = elem (i, i+k);
	      if (tmp != 0.)
		{
		  d.xdata (ii) = tmp;
		  d.xridx (ii++) = i;
		}
	    }
	}
      else if ( k < 0)
	{
	  for (octave_idx_type i = 0; i < ndiag; i++)
	    {
	      Complex tmp = elem (i-k, i);
	      if (tmp != 0.)
		{
		  d.xdata (ii) = tmp;
		  d.xridx (ii++) = i;
		}
	    }
	}
      else
	{
	  for (octave_idx_type i = 0; i < ndiag; i++)
	    {
	      Complex tmp = elem (i, i);
	      if (tmp != 0.)
		{
		  d.xdata (ii) = tmp;
		  d.xridx (ii++) = i;
		}
	    }
	}
    }
  else
    (*current_liboctave_error_handler) 
      ("diag: requested diagonal out of range");

  return d;
}

std::ostream&
operator << (std::ostream& os, const SparseComplexMatrix& a)
{
  octave_idx_type nc = a.cols ();

   // add one to the printed indices to go from
   //  zero-based to one-based arrays
   for (octave_idx_type j = 0; j < nc; j++)  {
      OCTAVE_QUIT;
      for (octave_idx_type i = a.cidx(j); i < a.cidx(j+1); i++) {
	os << a.ridx(i) + 1 << " "  << j + 1 << " ";
	octave_write_complex (os, a.data(i));
	os << "\n";
      }
   }

  return os;
}

std::istream&
operator >> (std::istream& is, SparseComplexMatrix& a)
{
  octave_idx_type nr = a.rows ();
  octave_idx_type nc = a.cols ();
  octave_idx_type nz = a.nnz ();

  if (nr < 1 || nc < 1)
    is.clear (std::ios::badbit);
  else
    {
      octave_idx_type itmp, jtmp, jold = 0;
      Complex tmp;
      octave_idx_type ii = 0;
       
      a.cidx (0) = 0;
      for (octave_idx_type i = 0; i < nz; i++)
	{
	  is >> itmp;
	  itmp--;
	  is >> jtmp;
	  jtmp--;
	  tmp = octave_read_complex (is);

	  if (is)
	    {
	      if (jold != jtmp)
		{
		  for (octave_idx_type j = jold; j < jtmp; j++)
		    a.cidx(j+1) = ii;
		  
		  jold = jtmp;
		}
	      a.data (ii) = tmp;
	      a.ridx (ii++) = itmp;
	    }
	  else
	    goto done;
	}

      for (octave_idx_type j = jold; j < nc; j++)
	a.cidx(j+1) = ii;
    }

 done:

  return is;
}

SparseComplexMatrix
operator * (const SparseComplexMatrix& m, const SparseMatrix& a)
{
  SparseComplexMatrix tmp (a);
  return m * tmp;
}

SparseComplexMatrix
operator * (const SparseMatrix& m, const SparseComplexMatrix& a)
{
  SparseComplexMatrix tmp (m);
  return tmp * a;
}

SparseComplexMatrix
operator * (const SparseComplexMatrix& m, const SparseComplexMatrix& a)
{
#ifdef HAVE_SPARSE_BLAS
  // XXX FIXME XXX Isn't there a sparse BLAS ??
#else
  // Use Andy's sparse matrix multiply function
  SPARSE_SPARSE_MUL (SparseComplexMatrix, Complex);
#endif
}

ComplexMatrix
operator * (const ComplexMatrix& m, const SparseMatrix& a)
{
  SparseComplexMatrix tmp (a);
  return m * tmp;
}

ComplexMatrix
operator * (const Matrix& m, const SparseComplexMatrix& a)
{
  ComplexMatrix tmp (m);
  return tmp * a;
}

ComplexMatrix
operator * (const ComplexMatrix& m, const SparseComplexMatrix& a)
{
#ifdef HAVE_SPARSE_BLAS
  // XXX FIXME XXX Isn't there a sparse BLAS ??
#else
  FULL_SPARSE_MUL (ComplexMatrix, Complex);
#endif
}

ComplexMatrix
operator * (const SparseComplexMatrix& m, const Matrix& a)
{
  ComplexMatrix tmp (a);
  return m * tmp;
}

ComplexMatrix
operator * (const SparseMatrix& m, const ComplexMatrix& a)
{
  SparseComplexMatrix tmp (m);
  return tmp * a;
}

ComplexMatrix
operator * (const SparseComplexMatrix& m, const ComplexMatrix& a)
{
#ifdef HAVE_SPARSE_BLAS
  // XXX FIXME XXX Isn't there a sparse BLAS ??
#else
  SPARSE_FULL_MUL (ComplexMatrix, Complex);
#endif
}

// XXX FIXME XXX -- it would be nice to share code among the min/max
// functions below.

#define EMPTY_RETURN_CHECK(T) \
  if (nr == 0 || nc == 0) \
    return T (nr, nc);

SparseComplexMatrix
min (const Complex& c, const SparseComplexMatrix& m)
{
  SparseComplexMatrix result;

  octave_idx_type nr = m.rows ();
  octave_idx_type nc = m.columns ();

  EMPTY_RETURN_CHECK (SparseComplexMatrix);

  if (abs(c) == 0.)
    return SparseComplexMatrix (nr, nc);
  else
    {
      result = SparseComplexMatrix (m);

      for (octave_idx_type j = 0; j < nc; j++)
	for (octave_idx_type i = m.cidx(j); i < m.cidx(j+1); i++)
	  result.data(i) = xmin(c, m.data(i));
    }
  
  return result;
}

SparseComplexMatrix
min (const SparseComplexMatrix& m, const Complex& c)
{
  return min (c, m);
}

SparseComplexMatrix
min (const SparseComplexMatrix& a, const SparseComplexMatrix& b)
{
  SparseComplexMatrix r;

  if ((a.rows() == b.rows()) && (a.cols() == b.cols())) 
    {
      octave_idx_type a_nr = a.rows ();
      octave_idx_type a_nc = a.cols ();

      octave_idx_type b_nr = b.rows ();
      octave_idx_type b_nc = b.cols ();

      if (a_nr == 0 || b_nc == 0 || a.nnz () == 0 || b.nnz () == 0)
	return SparseComplexMatrix (a_nr, a_nc);

      if (a_nr != b_nr || a_nc != b_nc)
	gripe_nonconformant ("min", a_nr, a_nc, b_nr, b_nc);
      else
	{
	  r = SparseComplexMatrix (a_nr, a_nc, (a.nnz () + b.nnz ()));
       
	  octave_idx_type jx = 0;
	  r.cidx (0) = 0;
	  for (octave_idx_type i = 0 ; i < a_nc ; i++)
	    {
	      octave_idx_type  ja = a.cidx(i);
	      octave_idx_type  ja_max = a.cidx(i+1);
	      bool ja_lt_max= ja < ja_max;
           
	      octave_idx_type  jb = b.cidx(i);
	      octave_idx_type  jb_max = b.cidx(i+1);
	      bool jb_lt_max = jb < jb_max;
           
	      while (ja_lt_max || jb_lt_max )
		{
		  OCTAVE_QUIT;
		  if ((! jb_lt_max) ||
                      (ja_lt_max && (a.ridx(ja) < b.ridx(jb))))
		    {
		      Complex tmp = xmin (a.data(ja), 0.);
		      if (tmp != 0.)
			{
			  r.ridx(jx) = a.ridx(ja);
			  r.data(jx) = tmp;
			  jx++;
			}
		      ja++;
		      ja_lt_max= ja < ja_max;
		    }
		  else if (( !ja_lt_max ) ||
			   (jb_lt_max && (b.ridx(jb) < a.ridx(ja)) ) )
		    {
		      Complex tmp = xmin (0., b.data(jb));
		      if (tmp != 0.)
			{
			  r.ridx(jx) = b.ridx(jb);
			  r.data(jx) = tmp;
			  jx++;
			}
		      jb++;
		      jb_lt_max= jb < jb_max;
		    }
		  else
		    {
		      Complex tmp = xmin (a.data(ja), b.data(jb));
		      if (tmp != 0.)
			{
                          r.data(jx) = tmp;
                          r.ridx(jx) = a.ridx(ja);
                          jx++;
			}
		      ja++;
		      ja_lt_max= ja < ja_max;
		      jb++;
		      jb_lt_max= jb < jb_max;
		    }
		}
	      r.cidx(i+1) = jx;
	    }
	  
	  r.maybe_compress ();
	}
    }
  else
    (*current_liboctave_error_handler) ("matrix size mismatch");

  return r;
}

SparseComplexMatrix
max (const Complex& c, const SparseComplexMatrix& m)
{
  SparseComplexMatrix result;

  octave_idx_type nr = m.rows ();
  octave_idx_type nc = m.columns ();

  EMPTY_RETURN_CHECK (SparseComplexMatrix);

  // Count the number of non-zero elements
  if (xmax(c, 0.) != 0.)
    {
      result = SparseComplexMatrix (nr, nc, c);
      for (octave_idx_type j = 0; j < nc; j++)
	for (octave_idx_type i = m.cidx(j); i < m.cidx(j+1); i++)
	  result.xdata(m.ridx(i) + j * nr) = xmax (c, m.data(i));
    }
  else
    result = SparseComplexMatrix (m);

  return result;
}

SparseComplexMatrix
max (const SparseComplexMatrix& m, const Complex& c)
{
  return max (c, m);
}

SparseComplexMatrix
max (const SparseComplexMatrix& a, const SparseComplexMatrix& b)
{
  SparseComplexMatrix r;

  if ((a.rows() == b.rows()) && (a.cols() == b.cols())) 
    {
      octave_idx_type a_nr = a.rows ();
      octave_idx_type a_nc = a.cols ();

      octave_idx_type b_nr = b.rows ();
      octave_idx_type b_nc = b.cols ();

      if (a_nr == 0 || b_nc == 0)
	return SparseComplexMatrix (a_nr, a_nc);
      if (a.nnz () == 0)
	return SparseComplexMatrix (b);
      if (b.nnz () == 0)
	return SparseComplexMatrix (a);

      if (a_nr != b_nr || a_nc != b_nc)
	gripe_nonconformant ("min", a_nr, a_nc, b_nr, b_nc);
      else
	{
	  r = SparseComplexMatrix (a_nr, a_nc, (a.nnz () + b.nnz ()));
       
	  octave_idx_type jx = 0;
	  r.cidx (0) = 0;
	  for (octave_idx_type i = 0 ; i < a_nc ; i++)
	    {
	      octave_idx_type  ja = a.cidx(i);
	      octave_idx_type  ja_max = a.cidx(i+1);
	      bool ja_lt_max= ja < ja_max;
           
	      octave_idx_type  jb = b.cidx(i);
	      octave_idx_type  jb_max = b.cidx(i+1);
	      bool jb_lt_max = jb < jb_max;
           
	      while (ja_lt_max || jb_lt_max )
		{
		  OCTAVE_QUIT;
		  if ((! jb_lt_max) ||
                      (ja_lt_max && (a.ridx(ja) < b.ridx(jb))))
		    {
		      Complex tmp = xmax (a.data(ja), 0.);
		      if (tmp != 0.)
			{
			  r.ridx(jx) = a.ridx(ja);
			  r.data(jx) = tmp;
			  jx++;
			}
		      ja++;
		      ja_lt_max= ja < ja_max;
		    }
		  else if (( !ja_lt_max ) ||
			   (jb_lt_max && (b.ridx(jb) < a.ridx(ja)) ) )
		    {
		      Complex tmp = xmax (0., b.data(jb));
		      if (tmp != 0.)
			{
			  r.ridx(jx) = b.ridx(jb);
			  r.data(jx) = tmp;
			  jx++;
			}
		      jb++;
		      jb_lt_max= jb < jb_max;
		    }
		  else
		    {
		      Complex tmp = xmax (a.data(ja), b.data(jb));
		      if (tmp != 0.)
			{
                          r.data(jx) = tmp;
                          r.ridx(jx) = a.ridx(ja);
                          jx++;
			}
		      ja++;
		      ja_lt_max= ja < ja_max;
		      jb++;
		      jb_lt_max= jb < jb_max;
		    }
		}
	      r.cidx(i+1) = jx;
	    }
	  
	  r.maybe_compress ();
	}
    }
  else
    (*current_liboctave_error_handler) ("matrix size mismatch");

  return r;
}

SPARSE_SMS_CMP_OPS (SparseComplexMatrix, 0.0, real, Complex, 
		   0.0, real)
SPARSE_SMS_BOOL_OPS (SparseComplexMatrix, Complex, 0.0)

SPARSE_SSM_CMP_OPS (Complex, 0.0, real, SparseComplexMatrix, 
		   0.0, real)
SPARSE_SSM_BOOL_OPS (Complex, SparseComplexMatrix, 0.0)

SPARSE_SMSM_CMP_OPS (SparseComplexMatrix, 0.0, real, SparseComplexMatrix, 
		     0.0, real)
SPARSE_SMSM_BOOL_OPS (SparseComplexMatrix, SparseComplexMatrix, 0.0)

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
