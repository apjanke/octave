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
along with this program; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <vector>

#include "config.h"
#include "quit.h"
#include "lo-ieee.h"
#include "lo-mappers.h"

#include "boolSparse.h"

// SparseBoolMatrix class.

bool
SparseBoolMatrix::operator == (const SparseBoolMatrix& a) const
{
  int nr = rows ();
  int nc = cols ();
  int nz = nnz ();
  int nr_a = a.rows ();
  int nc_a = a.cols ();
  int nz_a = a.nnz ();

  if (nr != nr_a || nc != nc_a || nz != nz_a)
    return false;

  for (int i = 0; i < nc + 1; i++)
    if (cidx(i) != a.cidx(i))
	return false;

  for (int i = 0; i < nz; i++)
    if (data(i) != a.data(i) || ridx(i) != a.ridx(i))
      return false;

  return true;
}

bool
SparseBoolMatrix::operator != (const SparseBoolMatrix& a) const
{
  return !(*this == a);
}

SparseBoolMatrix&
SparseBoolMatrix::insert (const SparseBoolMatrix& a, int r, int c)
{
  Sparse<bool>::insert (a, r, c);
  return *this;
}

SparseBoolMatrix
SparseBoolMatrix::concat (const SparseBoolMatrix& rb, const Array<int>& ra_idx)
{
  // Don't use numel to avoid all possiblity of an overflow
  if (rb.rows () > 0 && rb.cols () > 0)
    insert (rb, ra_idx(0), ra_idx(1));
  return *this;
}

// unary operations

SparseBoolMatrix
SparseBoolMatrix::operator ! (void) const
{
  int nr = rows ();
  int nc = cols ();
  int nz1 = nnz ();
  int nz2 = nr*nc - nz1;
   
  SparseBoolMatrix r (nr, nc, nz2);
   
  int ii = 0;
  int jj = 0;
  for (int i = 0; i < nc; i++)
    {
      for (int j = 0; j < nr; j++)
	{
	  if (jj < cidx(i+1) && ridx(jj) == j)
	    jj++;
	  else
	    {
	      r.data(ii) = true;
	      r.ridx(ii++) = j;
	    }
	}
      r.cidx (i) = ii;
    }

  return r;
}

// other operations

// XXX FIXME XXX Do these really belong here?  Maybe they should be
// in a base class?

SparseBoolMatrix
SparseBoolMatrix::all (int dim) const
{
  SPARSE_ALL_OP (dim);
}

SparseBoolMatrix
SparseBoolMatrix::any (int dim) const
{
  SPARSE_ANY_OP (dim);
}

boolMatrix
SparseBoolMatrix::matrix_value (void) const
{
  int nr = rows ();
  int nc = cols ();

  boolMatrix retval (nr, nc, false);
  for (int j = 0; j < nc; j++)
    for (int i = cidx(j); i < cidx(j+1); i++)
      retval.elem (ridx(i), j) = data (i);

  return retval;
}

std::ostream&
operator << (std::ostream& os, const SparseBoolMatrix& a)
{
  int nc = a.cols ();

   // add one to the printed indices to go from
   //  zero-based to one-based arrays
   for (int j = 0; j < nc; j++)  
     {
       OCTAVE_QUIT;
       for (int i = a.cidx(j); i < a.cidx(j+1); i++)
	 os << a.ridx(i) + 1 << " "  << j + 1 << " " << a.data(i) << "\n";
     }
   
  return os;
}

std::istream&
operator >> (std::istream& is, SparseBoolMatrix& a)
{
  int nr = a.rows ();
  int nc = a.cols ();
  int nz = a.nnz ();

  if (nr < 1 || nc < 1)
    is.clear (std::ios::badbit);
  else
    {
      int itmp, jtmp, jold = 0;
      bool tmp;
      int ii = 0;
       
      a.cidx (0) = 0;
      for (int i = 0; i < nz; i++)
	{
	  is >> itmp;
	  itmp--;
	  is >> jtmp;
	  jtmp--;
	  is >> tmp;
	  if (is)
	    {
	      if (jold != jtmp)
		{
		  for (int j = jold; j < jtmp; j++)
		    a.cidx(j+1) = ii;
		  
		  jold = jtmp;
		}
	      a.data (ii) = tmp;
	      a.ridx (ii++) = itmp;
	    }
	  else
	    goto done;
	}

      for (int j = jold; j < nc; j++)
	a.cidx(j+1) = ii;
    }

 done:

  return is;
}

SparseBoolMatrix
SparseBoolMatrix::squeeze (void) const 
{ 
  return Sparse<bool>::squeeze (); 
}

SparseBoolMatrix
SparseBoolMatrix::index (idx_vector& i, int resize_ok) const 
{ 
  return Sparse<bool>::index (i, resize_ok); 
}

SparseBoolMatrix
SparseBoolMatrix::index (idx_vector& i, idx_vector& j, int resize_ok) const 
{ 
  return Sparse<bool>::index (i, j, resize_ok); 
}
  
SparseBoolMatrix
SparseBoolMatrix::index (Array<idx_vector>& ra_idx, int resize_ok) const 
{ 
  return Sparse<bool>::index (ra_idx, resize_ok); 
}

SparseBoolMatrix
SparseBoolMatrix::reshape (const dim_vector& new_dims) const
{
  return Sparse<bool>::reshape (new_dims);
}

SparseBoolMatrix
SparseBoolMatrix::permute (const Array<int>& vec, bool inv) const
{
  return Sparse<bool>::permute (vec, inv);
}

SparseBoolMatrix
SparseBoolMatrix::ipermute (const Array<int>& vec) const
{
  return Sparse<bool>::ipermute (vec);
}

SPARSE_SMS_EQNE_OPS (SparseBoolMatrix, false, , bool, false, )
SPARSE_SMS_BOOL_OPS (SparseBoolMatrix, bool, false)

SPARSE_SSM_EQNE_OPS (bool, false, , SparseBoolMatrix, false, )
SPARSE_SSM_BOOL_OPS (bool, SparseBoolMatrix, false)

SPARSE_SMSM_EQNE_OPS (SparseBoolMatrix, false, , SparseBoolMatrix, false, )
SPARSE_SMSM_BOOL_OPS (SparseBoolMatrix, SparseBoolMatrix, false)


/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
