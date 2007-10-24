/*

Copyright (C) 2004, 2005, 2006, 2007 David Bateman and John W. Eaton
Copyright (C) 1996, 1997, 1999, 2000, 2001, 2002, 2003 John W. Eaton

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

#include <vector>

#include "lo-mappers.h"
#include "quit.h"

#include "defun-dld.h"
#include "error.h"
#include "gripes.h"
#include "oct-obj.h"
#include "lo-ieee.h"
#include "data-conv.h"
#include "ov-cx-mat.h"
#include "ov-cell.h"
#include "oct-sort.cc"

enum sortmode { UNDEFINED, ASCENDING, DESCENDING };

template <class T>
class
vec_index
{
public:
  T vec;
  octave_idx_type indx;
};

template <class T>
bool 
ascending_compare (T a, T b)
{
  return (a < b);
}

template <class T>
bool 
descending_compare (T a, T b)
{
  return (a > b);
}

template <class T>
bool 
ascending_compare (vec_index<T> *a, vec_index<T> *b)
{
  return (a->vec < b->vec);
}

template <class T>
bool 
descending_compare (vec_index<T> *a, vec_index<T> *b)
{
  return (a->vec > b->vec);
}

template <class T>
static octave_value
mx_sort (ArrayN<T> &m, int dim, sortmode mode = UNDEFINED)
{
  octave_value retval;

  dim_vector dv = m.dims ();

  if (m.length () < 1)
    return ArrayN<T> (dv);

  octave_idx_type ns = dv(dim);
  octave_idx_type iter = dv.numel () / ns;
  octave_idx_type stride = 1;
  for (int i = 0; i < dim; i++)
    stride *= dv(i);

  T *v = m.fortran_vec ();
  octave_sort<T> sort;

  if (mode == ASCENDING) 
    sort.set_compare (ascending_compare);
  else if (mode == DESCENDING)
    sort.set_compare (descending_compare);

  if (stride == 1)
    {
      for (octave_idx_type j = 0; j < iter; j++)
	{
	  sort.sort (v, ns);
	  v += ns;
	}
    }
  else
    {
      OCTAVE_LOCAL_BUFFER (T, vi, ns);
      for (octave_idx_type j = 0; j < iter; j++) 
	{
	   octave_idx_type offset = j;
	   octave_idx_type offset2 = 0;
	  while (offset >= stride)
	    {
	      offset -= stride;
	      offset2++;
	    }
	  offset += offset2 * stride * ns;
	  
	  for (octave_idx_type i = 0; i < ns; i++)
	    vi[i] = v[i*stride + offset];

	  sort.sort (vi, ns);
	      
	  for (octave_idx_type i = 0; i < ns; i++)
	    v[i*stride + offset] = vi[i];
	}
    }

  retval = m;

  return retval;
}

template <class T>
static octave_value_list
mx_sort_indexed (ArrayN<T> &m, int dim, sortmode mode = UNDEFINED)
{
  octave_value_list retval;

  dim_vector dv = m.dims ();

  if (m.length () < 1)
    {
      retval(1) = NDArray (dv);
      retval(0) = ArrayN<T> (dv);
      return retval;
    }

  octave_idx_type ns = dv(dim);
  octave_idx_type iter = dv.numel () / ns;
  octave_idx_type stride = 1;
  for (int i = 0; i < dim; i++)
    stride *= dv(i);

  T *v = m.fortran_vec ();
  octave_sort<vec_index<T> *> indexed_sort;

  if (mode == ASCENDING) 
    indexed_sort.set_compare (ascending_compare);
  else if (mode == DESCENDING)
    indexed_sort.set_compare (descending_compare);

  OCTAVE_LOCAL_BUFFER (vec_index<T> *, vi, ns);
  OCTAVE_LOCAL_BUFFER (vec_index<T>, vix, ns);

  for (octave_idx_type i = 0; i < ns; i++)
    vi[i] = &vix[i];

  NDArray idx (dv);
      
  if (stride == 1)
    {
      for (octave_idx_type j = 0; j < iter; j++)
	{
	   octave_idx_type offset = j * ns;

	  for (octave_idx_type i = 0; i < ns; i++)
	    {
	      vi[i]->vec = v[i];
	      vi[i]->indx = i + 1;
	    }

	  indexed_sort.sort (vi, ns);

	  for (octave_idx_type i = 0; i < ns; i++)
	    {
	      v[i] = vi[i]->vec;
	      idx(i + offset) = vi[i]->indx;
	    }
	  v += ns;
	}
    }
  else
    {
      for (octave_idx_type j = 0; j < iter; j++)
	{
	  octave_idx_type offset = j;
	  octave_idx_type offset2 = 0;
	  while (offset >= stride)
	    {
	      offset -= stride;
	      offset2++;
	    }
	  offset += offset2 * stride * ns;
	      
	  for (octave_idx_type i = 0; i < ns; i++)
	    {
	      vi[i]->vec = v[i*stride + offset];
	      vi[i]->indx = i + 1;
	    }

	  indexed_sort.sort (vi, ns);
	      
	  for (octave_idx_type i = 0; i < ns; i++)
	    {
	      v[i*stride+offset] = vi[i]->vec;
	      idx(i*stride+offset) = vi[i]->indx;
	    }
	}
    }

  retval(1) = idx;
  retval(0) = octave_value (m);

  return retval;
}

template <class T>
static octave_value
mx_sort_sparse (Sparse<T> &m, int dim, sortmode mode = UNDEFINED)
{
  octave_value retval;

  octave_idx_type nr = m.rows ();
  octave_idx_type nc = m.columns ();

  if (m.length () < 1)
    return Sparse<T> (nr, nc);

  if (dim > 0)
    {
      m = m.transpose ();
      nr = m.rows ();
      nc = m.columns ();
    }

  octave_sort<T> sort;

  if (mode == ASCENDING) 
    sort.set_compare (ascending_compare);
  else if (mode == DESCENDING)
    sort.set_compare (descending_compare);

  T *v = m.data ();
  octave_idx_type *cidx = m.cidx ();
  octave_idx_type *ridx = m.ridx ();

  for (octave_idx_type j = 0; j < nc; j++)
    {
      octave_idx_type ns = cidx [j + 1] - cidx [j];
      sort.sort (v, ns);

      octave_idx_type i;
      if (mode == ASCENDING) 
	{
	  for (i = 0; i < ns; i++)
	    if (ascending_compare (static_cast<T> (0), v [i]))
	      break;
	}
      else
	{
	  for (i = 0; i < ns; i++)
	    if (descending_compare (static_cast<T> (0), v [i]))
	      break;
	}
      for (octave_idx_type k = 0; k < i; k++)
	ridx [k] = k;
      for (octave_idx_type k = i; k < ns; k++)
	ridx [k] = k - ns + nr; 

      v += ns;
      ridx += ns;
    }

  if (dim > 0)
      m = m.transpose ();

  retval = m;

  return retval;
}

template <class T>
static octave_value_list
mx_sort_sparse_indexed (Sparse<T> &m, int dim, sortmode mode = UNDEFINED)
{
  octave_value_list retval;

  octave_idx_type nr = m.rows ();
  octave_idx_type nc = m.columns ();

  if (m.length () < 1)
    {
      retval (1) = NDArray (dim_vector (nr, nc));
      retval (0) = octave_value (SparseMatrix (nr, nc));
      return retval;
    }

  if (dim > 0)
    {
      m = m.transpose ();
      nr = m.rows ();
      nc = m.columns ();
    }

  octave_sort<vec_index<T> *> indexed_sort;

  if (mode == ASCENDING) 
    indexed_sort.set_compare (ascending_compare);
  else if (mode == DESCENDING)
    indexed_sort.set_compare (descending_compare);

  T *v = m.data ();
  octave_idx_type *cidx = m.cidx ();
  octave_idx_type *ridx = m.ridx ();

  OCTAVE_LOCAL_BUFFER (vec_index<T> *, vi, nr);
  OCTAVE_LOCAL_BUFFER (vec_index<T>, vix, nr);

  for (octave_idx_type i = 0; i < nr; i++)
    vi[i] = &vix[i];

  Matrix idx (nr, nc);

  for (octave_idx_type j = 0; j < nc; j++)
    {
      octave_idx_type ns = cidx [j + 1] - cidx [j];
      octave_idx_type offset = j * nr;

      if (ns == 0)
	{
	  for (octave_idx_type k = 0; k < nr; k++)
	    idx (offset + k) = k + 1;
	}
      else
	{
	  for (octave_idx_type i = 0; i < ns; i++)
	    {
	      vi[i]->vec = v[i];
	      vi[i]->indx = ridx[i] + 1;
	    }

	  indexed_sort.sort (vi, ns);

	  octave_idx_type i;
	  if (mode == ASCENDING) 
	    {
	      for (i = 0; i < ns; i++)
		if (ascending_compare (static_cast<T> (0), vi [i] -> vec))
		  break;
	    }
	  else
	    {
	      for (i = 0; i < ns; i++)
		if (descending_compare (static_cast<T> (0), vi [i] -> vec))
		  break;
	    }

	  octave_idx_type ii = 0;
	  octave_idx_type jj = i;
	  for (octave_idx_type k = 0; k < nr; k++)
	    {
	      if (ii < ns && ridx[ii] == k)
		ii++;
	      else
		idx (offset + jj++) = k + 1;
	    }

	  for (octave_idx_type k = 0; k < i; k++)
	    {
	      v [k] = vi [k] -> vec;
	      idx (k + offset) = vi [k] -> indx;
	      ridx [k] = k;
	    }

	  for (octave_idx_type k = i; k < ns; k++)
	    {
	      v [k] = vi [k] -> vec;
	      idx (k - ns + nr + offset) = vi [k] -> indx;
	      ridx [k] = k - ns + nr; 
	    }

	  v += ns;
	  ridx += ns;
	}
    }

  if (dim > 0)
    {
      m = m.transpose ();
      idx = idx.transpose ();
    }

  retval (1) = idx;
  retval(0) = octave_value (m);

  return retval;
}

// If we have IEEE 754 data format, then we can use the trick of
// casting doubles as unsigned eight byte integers, and with a little
// bit of magic we can automatically sort the NaN's correctly.

#if defined (HAVE_IEEE754_DATA_FORMAT)

static inline uint64_t
FloatFlip (uint64_t f)
{
  uint64_t mask
    = -static_cast<int64_t>(f >> 63) | 0x8000000000000000ULL;

  return f ^ mask;
}

static inline uint64_t
IFloatFlip (uint64_t f)
{
  uint64_t mask = ((f >> 63) - 1) | 0x8000000000000000ULL;

  return f ^ mask;
}

template <>
bool
ascending_compare (uint64_t a, 
		   uint64_t b)
{
  return (a < b);
}

template <>
bool
ascending_compare (vec_index<uint64_t> *a, 
		   vec_index<uint64_t> *b)
{
  return (a->vec < b->vec);
}

template <>
bool
descending_compare (uint64_t a, 
		    uint64_t b)
{
  return (a > b);
}

template <>
bool
descending_compare (vec_index<uint64_t> *a, 
		    vec_index<uint64_t> *b)
{
  return (a->vec > b->vec);
}

template class octave_sort<uint64_t>;
template class vec_index<uint64_t>;
template class octave_sort<vec_index<uint64_t> *>;

template <>
octave_value
mx_sort (ArrayN<double> &m, int dim, sortmode mode)
{
  octave_value retval;

  dim_vector dv = m.dims ();

  if (m.length () < 1)
    return ArrayN<double> (dv);

  octave_idx_type ns = dv(dim);
  octave_idx_type iter = dv.numel () / ns;
  octave_idx_type stride = 1;
  for (int i = 0; i < dim; i++)
    stride *= dv(i);

  double *v = m.fortran_vec ();

  uint64_t *p = reinterpret_cast<uint64_t *> (v);

  octave_sort<uint64_t> sort;

  if (mode == ASCENDING)
    sort.set_compare (ascending_compare);
  else if (mode == DESCENDING)
    sort.set_compare (descending_compare);

  if (stride == 1)
    {
      for (octave_idx_type j = 0; j < iter; j++)
	{
	  // Flip the data in the vector so that int compares on
	  // IEEE754 give the correct ordering.

	  for (octave_idx_type i = 0; i < ns; i++)
	    p[i] = FloatFlip (p[i]);
	      
	  sort.sort (p, ns);

	  // Flip the data out of the vector so that int compares
	  // on IEEE754 give the correct ordering.

	  for (octave_idx_type i = 0; i < ns; i++)
	    p[i] = IFloatFlip (p[i]);

	  // There are two representations of NaN.  One will be
	  // sorted to the beginning of the vector and the other
	  // to the end.  If it will be sorted incorrectly, fix
	  // things up.

	  if (lo_ieee_signbit (octave_NaN))
	    {
	      if (mode == UNDEFINED || mode == ASCENDING)
		{
		  octave_idx_type i = 0;
		  double *vtmp = reinterpret_cast<double *> (p);
		  while (xisnan (vtmp[i++]) && i < ns);
		  for (octave_idx_type l = 0; l < ns - i + 1; l++)
		    vtmp[l] = vtmp[l+i-1];
		  for (octave_idx_type l = ns - i + 1; l < ns; l++)
		    vtmp[l] = octave_NaN;
		}
	      else
		{
		  octave_idx_type i = ns;
		  double *vtmp = reinterpret_cast<double *> (p);
		  while (xisnan (vtmp[--i]) && i > 0);
		  for (octave_idx_type l = i; l >= 0; l--)
		    vtmp[l-i+ns-1] = vtmp[l];
		  for (octave_idx_type l = 0; l < ns - i - 1; l++)
		    vtmp[l] = octave_NaN;
		}
	    }

	  p += ns;
	}
    }
  else
    {
      OCTAVE_LOCAL_BUFFER (uint64_t, vi, ns);

      for (octave_idx_type j = 0; j < iter; j++)
	{
	  octave_idx_type offset = j;
	  octave_idx_type offset2 = 0;
	  while (offset >= stride)
	    {
	      offset -= stride;
	      offset2++;
	    }
	  offset += offset2 * stride * ns;

	  // Flip the data in the vector so that int compares on
	  // IEEE754 give the correct ordering.

	  for (octave_idx_type i = 0; i < ns; i++)
	    vi[i] = FloatFlip (p[i*stride + offset]);

	  sort.sort (vi, ns);

	  // Flip the data out of the vector so that int compares
	  // on IEEE754 give the correct ordering.

	  for (octave_idx_type i = 0; i < ns; i++)
	    p[i*stride + offset] = IFloatFlip (vi[i]);
	      
	  // There are two representations of NaN. One will be
	  // sorted to the beginning of the vector and the other
	  // to the end. If it will be sorted to the beginning,
	  // fix things up.

	  if (lo_ieee_signbit (octave_NaN))
	    {
	      if (mode == UNDEFINED || mode == ASCENDING)
		{
		   octave_idx_type i = 0;
		  while (xisnan (v[i++*stride + offset]) && i < ns);
		  for (octave_idx_type l = 0; l < ns - i + 1; l++)
		    v[l*stride + offset] = v[(l+i-1)*stride + offset];
		  for (octave_idx_type l = ns - i + 1; l < ns; l++)
		    v[l*stride + offset] = octave_NaN;
		}
	      else
		{
		   octave_idx_type i = ns;
		  while (xisnan (v[--i*stride + offset]) && i > 0);
		  for (octave_idx_type l = i; l >= 0; l--)
		    v[(l-i+ns-1)*stride + offset] = v[l*stride + offset];
		  for (octave_idx_type l = 0; l < ns - i - 1; l++)
		    v[l*stride + offset] = octave_NaN;
		}
	    }
	}
    }

  retval = m;

  return retval;
}

// Should other overloaded functions have their static keywords removed?
template <>
octave_value_list
mx_sort_indexed (ArrayN<double> &m, int dim, sortmode mode)
{
  octave_value_list retval;

  dim_vector dv = m.dims ();

  if (m.length () < 1)
    {
      retval(1) = NDArray (dv);
      retval(0) = ArrayN<double> (dv);
      return retval;
    }

  octave_idx_type ns = dv(dim);
  octave_idx_type iter = dv.numel () / ns;
  octave_idx_type stride = 1;
  for (int i = 0; i < dim; i++)
    stride *= dv(i);

  double *v = m.fortran_vec ();

  uint64_t *p = reinterpret_cast<uint64_t *> (v);

  octave_sort<vec_index<uint64_t> *> indexed_sort;

  if (mode == ASCENDING)
    indexed_sort.set_compare (ascending_compare);
  else if (mode == DESCENDING)
    indexed_sort.set_compare (descending_compare);

  OCTAVE_LOCAL_BUFFER (vec_index<uint64_t> *, vi, ns);
  OCTAVE_LOCAL_BUFFER (vec_index<uint64_t>, vix, ns);
  
  for (octave_idx_type i = 0; i < ns; i++)
    vi[i] = &vix[i];

  NDArray idx (dv);
      
  for (octave_idx_type j = 0; j < iter; j++)
    {
      octave_idx_type offset = j;
      octave_idx_type offset2 = 0;
      while (offset >= stride)
	{
	  offset -= stride;
	  offset2++;
	}
      offset += offset2 * stride * ns;

      // Flip the data in the vector so that int compares on
      // IEEE754 give the correct ordering.

      for (octave_idx_type i = 0; i < ns; i++)
	{
	  vi[i]->vec = FloatFlip (p[i*stride + offset]);
	  vi[i]->indx = i + 1;
	}

      indexed_sort.sort (vi, ns);

      // Flip the data out of the vector so that int compares on
      // IEEE754 give the correct ordering

      for (octave_idx_type i = 0; i < ns; i++)
	{
	  p[i*stride + offset] = IFloatFlip (vi[i]->vec);
	  idx(i*stride + offset) = vi[i]->indx;
	}

      // There are two representations of NaN.  One will be sorted
      // to the beginning of the vector and the other to the end.
      // If it will be sorted to the beginning, fix things up.

      if (lo_ieee_signbit (octave_NaN))
	{
	  if (mode == UNDEFINED || mode == ASCENDING)
	    {
	      octave_idx_type i = 0;
	      while (xisnan (v[i++*stride+offset]) && i < ns);
	      OCTAVE_LOCAL_BUFFER (double, itmp, i - 1);
	      for (octave_idx_type l = 0; l < i -1; l++)
		itmp[l] = idx(l*stride + offset);
	      for (octave_idx_type l = 0; l < ns - i + 1; l++)
		{
		  v[l*stride + offset] = v[(l+i-1)*stride + offset];
		  idx(l*stride + offset) = idx((l+i-1)*stride + offset);
		}
	      for (octave_idx_type k = 0, l = ns - i + 1; l < ns; l++, k++)
		{
		  v[l*stride + offset] = octave_NaN;
		  idx(l*stride + offset) = itmp[k];
		}
	    }
	  else 
	    {
	      octave_idx_type i = ns;
	      while (xisnan (v[--i*stride+offset]) && i > 0);
	      OCTAVE_LOCAL_BUFFER (double, itmp, ns - i - 1);
	      for (octave_idx_type l = 0; l < ns - i -1; l++)
		itmp[l] = idx((l+i+1)*stride + offset);
	      for (octave_idx_type l = i; l >= 0; l--)
		{
		  v[(l-i+ns-1)*stride + offset] = v[l*stride + offset];
		  idx((l-i+ns-1)*stride + offset) = idx(l*stride + offset);
		}
	      for (octave_idx_type k = 0, l = 0; l < ns - i - 1; l++, k++)
		{
		  v[l*stride + offset] = octave_NaN;
		  idx(l*stride + offset) = itmp[k];
		}
	    }
	}
    }

  retval(1) = idx;
  retval(0) = m;

  return retval;
}

#else

template <>
bool
ascending_compare (double a, double b)
{
  return (xisnan (b) || (a < b));
}

template <>
bool
ascending_compare (vec_index<double> *a, vec_index<double> *b)
{
  return (xisnan (b->vec) || (a->vec < b->vec));
}

template <>
bool
descending_compare (double a, double b)
{
  return (xisnan (a) || (a > b));
}

template <>
bool
descending_compare (vec_index<double> *a, vec_index<double> *b)
{
  return (xisnan (a->vec) || (a->vec > b->vec));
}

template class octave_sort<double>;
template class vec_index<double>;
template class octave_sort<vec_index<double> *>;

#if !defined (CXX_NEW_FRIEND_TEMPLATE_DECL)
static octave_value_list
mx_sort (ArrayN<double> &m, int dim, sortmode mode);

static octave_value_list
mx_sort_indexed (ArrayN<double> &m, int dim, sortmode mode);
#endif
#endif

#if !defined (CXX_NEW_FRIEND_TEMPLATE_DECL)
static octave_value_list
mx_sort_sparse (Sparse<double> &m, int dim, sortmode mode);

static octave_value_list
mx_sort_sparse_indexed (Sparse<double> &m, int dim, sortmode mode);
#endif

// std::abs(Inf) returns NaN!!
static inline double
xabs (const Complex& x)
{
  return (xisinf (x.real ()) || xisinf (x.imag ())) ? octave_Inf : abs (x);
}

template <>
bool
ascending_compare (Complex a, Complex b)
{
  return (xisnan (b) || (xabs (a) < xabs (b)) || ((xabs (a) == xabs (b))
	      && (arg (a) < arg (b))));
}

bool
operator < (const Complex a, const Complex b)
{
  return (xisnan (b) || (xabs (a) < xabs (b)) || ((xabs (a) == xabs (b))
	      && (arg (a) < arg (b))));
}

template <>
bool
descending_compare (Complex a, Complex b)
{
  return (xisnan (a) || (xabs (a) > xabs (b)) || ((xabs (a) == xabs (b))
	      && (arg (a) > arg (b))));
}

bool
operator > (const Complex a, const Complex b)
{
  return (xisnan (a) || (xabs (a) > xabs (b)) || ((xabs (a) == xabs (b))
	      && (arg (a) > arg (b))));
}

template <>
bool
ascending_compare (vec_index<Complex> *a, vec_index<Complex> *b)
{
  return (xisnan (b->vec)
	  || (xabs (a->vec) < xabs (b->vec))
	  || ((xabs (a->vec) == xabs (b->vec))
	      && (arg (a->vec) < arg (b->vec))));
}

template <>
bool
descending_compare (vec_index<Complex> *a, vec_index<Complex> *b)
{
  return (xisnan (a->vec)
	  || (xabs (a->vec) > xabs (b->vec))
	  || ((xabs (a->vec) == xabs (b->vec))
	      && (arg (a->vec) > arg (b->vec))));
}

template class octave_sort<Complex>;
template class vec_index<Complex>;
template class octave_sort<vec_index<Complex> *>;

#if !defined (CXX_NEW_FRIEND_TEMPLATE_DECL)
static octave_value_list
mx_sort (ArrayN<Complex> &m, int dim, sortmode mode);

static octave_value_list
mx_sort_indexed (ArrayN<Complex> &m, int dim, sortmode mode);

static octave_value_list
mx_sort_sparse (Sparse<Complex> &m, int dim, sortmode mode);

static octave_value_list
mx_sort_sparse_indexed (Sparse<Complex> &m, int dim, sortmode mode);
#endif

template class octave_sort<char>;
template class vec_index<char>;
template class octave_sort<vec_index<char> *>;

#if !defined (CXX_NEW_FRIEND_TEMPLATE_DECL)
bool
ascending_compare (char a, char b);

bool
ascending_compare (vec_index<char> *a, vec_index<char> *b);

bool
descending_compare (char a, char b);

bool
descending_compare (vec_index<char> *a, vec_index<char> *b);

static octave_value_list
mx_sort (ArrayN<char> &m, int dim, sortmode mode);

static octave_value_list
mx_sort_indexed (ArrayN<char> &m, int dim, sortmode mode);
#endif

template class octave_sort<octave_int8>;
template class vec_index<octave_int8>;
template class octave_sort<vec_index<octave_int8> *>;

#if !defined (CXX_NEW_FRIEND_TEMPLATE_DECL)
bool
ascending_compare (octave_int8 a, octave_int8 b);

bool
ascending_compare (vec_index<octave_int8> *a, vec_index<octave_int8> *b);

bool
descending_compare (octave_int8 a, octave_int8 b);

bool
descending_compare (vec_index<octave_int8> *a, vec_index<octave_int8> *b);

static octave_value_list
mx_sort (ArrayN<octave_int8> &m, int dim, sortmode mode);

static octave_value_list
mx_sort_indexed (ArrayN<octave_int8> &m, int dim, sortmode mode);
#endif

template class octave_sort<octave_uint8>;
template class vec_index<octave_uint8>;
template class octave_sort<vec_index<octave_uint8> *>;

#if !defined (CXX_NEW_FRIEND_TEMPLATE_DECL)
bool
ascending_compare (octave_uint8 a, octave_uint8 b);

bool
ascending_compare (vec_index<octave_uint8> *a, vec_index<octave_uint8> *b);

bool
descending_compare (octave_uint8 a, octave_uint8 b);

bool
descending_compare (vec_index<octave_uint8> *a, vec_index<octave_uint8> *b);

static octave_value_list
mx_sort (ArrayN<octave_uint8> &m, int dim, sortmode mode);

static octave_value_list
mx_sort_indexed (ArrayN<octave_uint8> &m, int dim, sortmode mode);
#endif

template class octave_sort<octave_int16>;
template class vec_index<octave_int16>;
template class octave_sort<vec_index<octave_int16> *>;

#if !defined (CXX_NEW_FRIEND_TEMPLATE_DECL)
bool
ascending_compare (octave_int16 a, octave_int16 b);

bool
ascending_compare (vec_index<octave_int16> *a, vec_index<octave_int16> *b);

bool
descending_compare (octave_int16 a, octave_int16 b);

bool
descending_compare (vec_index<octave_int16> *a, vec_index<octave_int16> *b);

static octave_value_list
mx_sort (ArrayN<octave_int16> &m, int dim, sortmode mode);

static octave_value_list
mx_sort_indexed (ArrayN<octave_int16> &m, int dim, sortmode mode);
#endif

template class octave_sort<octave_uint16>;
template class vec_index<octave_uint16>;
template class octave_sort<vec_index<octave_uint16> *>;

#if !defined (CXX_NEW_FRIEND_TEMPLATE_DECL)
bool
ascending_compare (octave_uint16 a, octave_uint16 b);

bool
ascending_compare (vec_index<octave_uint16> *a, vec_index<octave_uint16> *b);

bool
descending_compare (octave_uint16 a, octave_uint16 b);

bool
descending_compare (vec_index<octave_uint16> *a, vec_index<octave_uint16> *b);

static octave_value_list
mx_sort (ArrayN<octave_uint16> &m, int dim, sortmode mode);

static octave_value_list
mx_sort_indexed (ArrayN<octave_uint16> &m, int dim, sortmode mode);
#endif

template class octave_sort<octave_int32>;
template class vec_index<octave_int32>;
template class octave_sort<vec_index<octave_int32> *>;

#if !defined (CXX_NEW_FRIEND_TEMPLATE_DECL)
bool
ascending_compare (octave_int32 a, octave_int32 b);

bool
ascending_compare (vec_index<octave_int32> *a, vec_index<octave_int32> *b);

bool
descending_compare (octave_int32 a, octave_int32 b);

bool
descending_compare (vec_index<octave_int32> *a, vec_index<octave_int32> *b);

static octave_value_list
mx_sort (ArrayN<octave_int32> &m, int dim, sortmode mode);

static octave_value_list
mx_sort_indexed (ArrayN<octave_int32> &m, int dim, sortmode mode);
#endif

template class octave_sort<octave_uint32>;
template class vec_index<octave_uint32>;
template class octave_sort<vec_index<octave_uint32> *>;

#if !defined (CXX_NEW_FRIEND_TEMPLATE_DECL)
bool
ascending_compare (octave_uint32 a, octave_uint32 b);

bool
ascending_compare (vec_index<octave_uint32> *a, vec_index<octave_uint32> *b);

bool
descending_compare (octave_uint32 a, octave_uint32 b);

bool
descending_compare (vec_index<octave_uint32> *a, vec_index<octave_uint32> *b);

static octave_value_list
mx_sort (ArrayN<octave_uint32> &m, int dim, sortmode mode);

static octave_value_list
mx_sort_indexed (ArrayN<octave_uint32> &m, int dim, sortmode mode);
#endif

template class octave_sort<octave_int64>;
template class vec_index<octave_int64>;
template class octave_sort<vec_index<octave_int64> *>;

#if !defined (CXX_NEW_FRIEND_TEMPLATE_DECL)
bool
ascending_compare (octave_int64 a, octave_int64 b);

bool
ascending_compare (vec_index<octave_int64> *a, vec_index<octave_int64> *b);

bool
descending_compare (octave_int64 a, octave_int64 b);

bool
descending_compare (vec_index<octave_int64> *a, vec_index<octave_int64> *b);

static octave_value_list
mx_sort (ArrayN<octave_int64> &m, int dim, sortmode mode);

static octave_value_list
mx_sort_indexed (ArrayN<octave_int64> &m, int dim, sortmode mode);
#endif

template class octave_sort<octave_uint64>;
template class vec_index<octave_uint64>;
template class octave_sort<vec_index<octave_uint64> *>;

#if !defined (CXX_NEW_FRIEND_TEMPLATE_DECL)
bool
ascending_compare (octave_uint64 a, octave_uint64 b);

bool
ascending_compare (vec_index<octave_uint64> *a, vec_index<octave_uint64> *b);

bool
descending_compare (octave_uint64 a, octave_uint64 b);

bool
descending_compare (vec_index<octave_uint64> *a, vec_index<octave_uint64> *b);

static octave_value_list
mx_sort (ArrayN<octave_uint64> &m, int dim, sortmode mode);

static octave_value_list
mx_sort_indexed (ArrayN<octave_uint64> &m, int dim, sortmode mode);
#endif

template <>
bool
ascending_compare (vec_index<octave_value> *a, vec_index<octave_value> *b)
{
  return (a->vec.string_value () < b->vec.string_value ());
}

template <>
bool
descending_compare (vec_index<octave_value> *a, vec_index<octave_value> *b)
{
  return (a->vec.string_value () > b->vec.string_value ());
}

template class vec_index<octave_value>;
template class octave_sort<vec_index<octave_value> *>;

#if !defined (CXX_NEW_FRIEND_TEMPLATE_DECL)
static octave_value_list
mx_sort_indexed (ArrayN<octave_value> &m, int dim, sortmode mode);
#endif

DEFUN_DLD (sort, args, nargout,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {[@var{s}, @var{i}] =} sort (@var{x})\n\
@deftypefnx {Loadable Function} {[@var{s}, @var{i}] =} sort (@var{x}, @var{dim})\n\
@deftypefnx {Loadable Function} {[@var{s}, @var{i}] =} sort (@var{x}, @var{mode})\n\
@deftypefnx {Loadable Function} {[@var{s}, @var{i}] =} sort (@var{x}, @var{dim}, @var{mode})\n\
Return a copy of @var{x} with the elements arranged in increasing\n\
order.  For matrices, @code{sort} orders the elements in each column.\n\
\n\
For example,\n\
\n\
@example\n\
@group\n\
sort ([1, 2; 2, 3; 3, 1])\n\
     @result{}  1  1\n\
         2  2\n\
         3  3\n\
@end group\n\
@end example\n\
\n\
The @code{sort} function may also be used to produce a matrix\n\
containing the original row indices of the elements in the sorted\n\
matrix.  For example,\n\
\n\
@example\n\
@group\n\
[s, i] = sort ([1, 2; 2, 3; 3, 1])\n\
     @result{} s = 1  1\n\
            2  2\n\
            3  3\n\
     @result{} i = 1  3\n\
            2  1\n\
            3  2\n\
@end group\n\
@end example\n\
\n\
If the optional argument @var{dim} is given, then the matrix is sorted\n\
along the dimension defined by @var{dim}. The optional argument @code{mode}\n\
defines the order in which the values will be sorted. Valid values of\n\
@code{mode} are `ascend' or `descend'.\n\
\n\
For equal elements, the indices are such that the equal elements are listed\n\
in the order that appeared in the original list.\n\
\n\
The @code{sort} function may also be used to sort strings and cell arrays\n\
of strings, in which case the dictionary order of the strings is used.\n\
\n\
The algorithm used in @code{sort} is optimized for the sorting of partially\n\
ordered lists.\n\
@end deftypefn")
{
  octave_value_list retval;

  int nargin = args.length ();
  sortmode smode = ASCENDING;

  if (nargin < 1 || nargin > 3)
    {
      print_usage ();
      return retval;
    }

  bool return_idx = nargout > 1;

  octave_value arg = args(0);

  int dim = 0;
  if (nargin > 1)
    {
      if (args(1).is_string ())
	{
	  std::string mode = args(1).string_value();
	  if (mode == "ascend")
	    smode = ASCENDING;
	  else if (mode == "descend")
	    smode = DESCENDING;
	  else
	    {
	      error ("sort: mode must be either \"ascend\" or \"descend\"");
	      return retval;
	    }
	}
      else
	dim = args(1).nint_value () - 1;
    }

  if (nargin > 2)
    {
      if (args(1).is_string ())
	{
	  print_usage ();
	  return retval;
	}

      if (! args(2).is_string ())
	{
	  error ("sort: mode must be a string");
	  return retval;
	}
      std::string mode = args(2).string_value();
      if (mode == "ascend")
	smode = ASCENDING;
      else if (mode == "descend")
	smode = DESCENDING;
      else
	{
	  error ("sort: mode must be either \"ascend\" or \"descend\"");
	  return retval;
	}
    }

  dim_vector dv = arg.dims ();
  if (error_state)
    {
      gripe_wrong_type_arg ("sort", arg);
      return retval;
    }
  if (nargin == 1 || args(1).is_string ())
    {
      // Find first non singleton dimension
      for (int i = 0; i < dv.length (); i++)
	if (dv(i) > 1)
	  {
	    dim = i;
	    break;
	  }
    }
  else
    {
      if (dim < 0 || dim > dv.length () - 1)
	{
	  error ("sort: dim must be a valid dimension");
	  return retval;
	}
    }

  // FIXME Perhaps sort should be made a method of the octave_value classes
  // and then the mess of if statements both might be replaced with 
  //   retval = arg.sort (dim, smode, return_idx);

  if (arg.is_real_type ())
    {
      if (arg.is_sparse_type ())
	{
	  SparseMatrix m = arg.sparse_matrix_value ();

	  if (! error_state)
	    {
	      if (return_idx)
		retval = mx_sort_sparse_indexed (m, dim, smode);
	      else
		retval = mx_sort_sparse (m, dim, smode);
	    }
	}
      else if (arg.is_int8_type ())
	{
	  int8NDArray m = arg.int8_array_value ();
	  if (! error_state)
	    {
	      if (return_idx)
		retval = mx_sort_indexed (m, dim, smode);
	      else
		retval = mx_sort (m, dim, smode);
	    }
	}
      else if (arg.is_uint8_type ())
	{
	  uint8NDArray m = arg.uint8_array_value ();
	  if (! error_state)
	    {
	      if (return_idx)
		retval = mx_sort_indexed (m, dim, smode);
	      else
		retval = mx_sort (m, dim, smode);
	    }
	}
      else if (arg.is_int16_type ())
	{
	  int16NDArray m = arg.int16_array_value ();
	  if (! error_state)
	    {
	      if (return_idx)
		retval = mx_sort_indexed (m, dim, smode);
	      else
		retval = mx_sort (m, dim, smode);
	    }
	}
      else if (arg.is_uint16_type ())
	{
	  uint16NDArray m = arg.uint16_array_value ();
	  if (! error_state)
	    {
	      if (return_idx)
		retval = mx_sort_indexed (m, dim, smode);
	      else
		retval = mx_sort (m, dim, smode);
	    }
	}
      else if (arg.is_int32_type ())
	{
	  int32NDArray m = arg.int32_array_value ();
	  if (! error_state)
	    {
	      if (return_idx)
		retval = mx_sort_indexed (m, dim, smode);
	      else
		retval = mx_sort (m, dim, smode);
	    }
	}
      else if (arg.is_uint32_type ())
	{
	  uint32NDArray m = arg.uint32_array_value ();
	  if (! error_state)
	    {
	      if (return_idx)
		retval = mx_sort_indexed (m, dim, smode);
	      else
		retval = mx_sort (m, dim, smode);
	    }
	}
      else if (arg.is_int64_type ())
	{
	  int64NDArray m = arg.int64_array_value ();
	  if (! error_state)
	    {
	      if (return_idx)
		retval = mx_sort_indexed (m, dim, smode);
	      else
		retval = mx_sort (m, dim, smode);
	    }
	}
      else if (arg.is_uint64_type ())
	{
	  uint64NDArray m = arg.uint64_array_value ();
	  if (! error_state)
	    {
	      if (return_idx)
		retval = mx_sort_indexed (m, dim, smode);
	      else
		retval = mx_sort (m, dim, smode);
	    }
	}
      else
	{
	  NDArray m = arg.array_value ();

	  if (! error_state)
	    {
#ifdef HAVE_IEEE754_DATA_FORMAT
	      // As operator > gives the right result, can special case here
	      if (! return_idx && smode == ASCENDING)
		retval = mx_sort (m, dim);
	      else
#endif
		{
		  if (return_idx)
		    retval = mx_sort_indexed (m, dim, smode);
		  else
		    retval = mx_sort (m, dim, smode);
		}
	    }
	}
    }
  else if (arg.is_complex_type ())
    {
      if (arg.is_sparse_type ())
	{
	  SparseComplexMatrix cm = arg.sparse_complex_matrix_value ();

	  if (! error_state)
	    {
	      if (return_idx)
		retval = mx_sort_sparse_indexed (cm, dim, smode);
	      else
		retval = mx_sort_sparse (cm, dim, smode);
	    }
	}
      else
	{
	  ComplexNDArray cm = arg.complex_array_value ();

	  if (! error_state)
	    {
	      // The indexed version seems to be slightly faster
	      retval = mx_sort_indexed (cm, dim, smode);
	    }
	}
    }
  else if (arg.is_string ())
    {
      charNDArray chm = arg.char_array_value ();

      if (! error_state)
	{
	  // As operator > gives the right result, can special case here
	  if (! return_idx && smode == ASCENDING)
	    retval = mx_sort (chm, dim);
	  else
	    {
	      if (return_idx)
		retval = mx_sort_indexed (chm, dim, smode);
	      else
		retval = mx_sort (chm, dim, smode);
	    }

	  // FIXME It would have been better to call 
	  // "octave_value(m, true)" but how can that be done 
	  // within the template
	  retval(0) = retval(0).convert_to_str (false, true);
	}
    }
  else if (arg.is_cell ())
    {
      Cell cellm = arg.cell_value ();

      // Need to check that all elements are strings
      for (octave_idx_type i = 0; i < cellm.numel (); i++)
	if (! cellm(i).is_string ())
	  {
	    gripe_wrong_type_arg ("sort", arg);
	    break;
	  }

      // Don't have unindexed version as ">" operator doesn't return bool
      if (!error_state)
	retval = mx_sort_indexed (cellm, dim, smode);
    }
  else
    gripe_wrong_type_arg ("sort", arg);

  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
