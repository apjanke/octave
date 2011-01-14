/*

Copyright (C) 2008-2011 David Bateman

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

#include "defun-dld.h"
#include "error.h"
#include "gripes.h"
#include "oct-obj.h"
#include "utils.h"

DEFUN_DLD (rcond, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{c} =} rcond (@var{a})\n\
Compute the 1-norm estimate of the reciprocal condition as returned\n\
by @sc{lapack}.  If the matrix is well-conditioned then @var{c} will be near\n\
1 and if the matrix is poorly conditioned it will be close to zero.\n\
\n\
The matrix @var{a} must not be sparse.  If the matrix is sparse then\n\
@code{condest (@var{a})} or @code{rcond (full (@var{a}))} should be used\n\
instead.\n\
@seealso{inv}\n\
@end deftypefn")
{
  octave_value retval;

  int nargin = args.length ();

  if (nargin != 1)
    print_usage ();
  else if (args(0).is_sparse_type ())
    error ("rcond: for sparse matrices use 'rcond (full (a))' or 'condest (a)' instead");
  else if (args(0).is_single_type ())
    {
      if (args(0).is_complex_type ())
        {
          FloatComplexMatrix m = args(0).float_complex_matrix_value ();
          MatrixType mattyp;
          retval = m.rcond (mattyp);
          args(0).matrix_type (mattyp);
        }
      else
        {
          FloatMatrix m = args(0).float_matrix_value ();
          MatrixType mattyp;
          retval = m.rcond (mattyp);
          args(0).matrix_type (mattyp);
        }
    }
  else if (args(0).is_complex_type ())
    {
      ComplexMatrix m = args(0).complex_matrix_value ();
      MatrixType mattyp;
      retval = m.rcond (mattyp);
      args(0).matrix_type (mattyp);
    }
  else
    {
      Matrix m = args(0).matrix_value ();
      MatrixType mattyp;
      retval = m.rcond (mattyp);
      args(0).matrix_type (mattyp);
    }

  return retval;
}
