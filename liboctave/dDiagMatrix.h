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

#if !defined (octave_DiagMatrix_h)
#define octave_DiagMatrix_h 1

#include "MDiagArray2.h"

#include "dRowVector.h"
#include "dColVector.h"

#include "mx-defs.h"

class
OCTAVE_API
DiagMatrix : public MDiagArray2<double>
{
friend class SVD;
friend class ComplexSVD;

public:

  DiagMatrix (void) : MDiagArray2<double> () { }

  DiagMatrix (octave_idx_type r, octave_idx_type c) : MDiagArray2<double> (r, c) { }

  DiagMatrix (octave_idx_type r, octave_idx_type c, double val) : MDiagArray2<double> (r, c, val) { }

  DiagMatrix (const DiagMatrix& a) : MDiagArray2<double> (a) { }

  DiagMatrix (const MDiagArray2<double>& a) : MDiagArray2<double> (a) { }

  explicit DiagMatrix (const RowVector& a) : MDiagArray2<double> (a) { }

  explicit DiagMatrix (const ColumnVector& a) : MDiagArray2<double> (a) { }

  DiagMatrix& operator = (const DiagMatrix& a)
    {
      MDiagArray2<double>::operator = (a);
      return *this;
    }

  bool operator == (const DiagMatrix& a) const;
  bool operator != (const DiagMatrix& a) const;

  DiagMatrix& fill (double val);
  DiagMatrix& fill (double val, octave_idx_type beg, octave_idx_type end);
  DiagMatrix& fill (const ColumnVector& a);
  DiagMatrix& fill (const RowVector& a);
  DiagMatrix& fill (const ColumnVector& a, octave_idx_type beg);
  DiagMatrix& fill (const RowVector& a, octave_idx_type beg);

  DiagMatrix transpose (void) const;

  friend DiagMatrix real (const ComplexDiagMatrix& a);
  friend DiagMatrix imag (const ComplexDiagMatrix& a);

  // resize is the destructive analog for this one

  Matrix extract (octave_idx_type r1, octave_idx_type c1, octave_idx_type r2, octave_idx_type c2) const;

  // extract row or column i.

  RowVector row (octave_idx_type i) const;
  RowVector row (char *s) const;

  ColumnVector column (octave_idx_type i) const;
  ColumnVector column (char *s) const;

  DiagMatrix inverse (void) const;
  DiagMatrix inverse (int& info) const;

  // other operations

  ColumnVector diag (void) const;
  ColumnVector diag (octave_idx_type k) const;

  // i/o

  friend std::ostream& operator << (std::ostream& os, const DiagMatrix& a);

private:

  DiagMatrix (double *d, octave_idx_type nr, octave_idx_type nc) : MDiagArray2<double> (d, nr, nc) { }
};

// diagonal matrix by diagonal matrix -> diagonal matrix operations

DiagMatrix
operator * (const DiagMatrix& a, const DiagMatrix& b);

MDIAGARRAY2_FORWARD_DEFS (MDiagArray2, DiagMatrix, double)

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
