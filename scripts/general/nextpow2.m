## Copyright (C) 1995, 1996, 1997, 1999, 2000, 2002, 2004, 2005, 2006,
##               2007, 2009 Kurt Hornik
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or (at
## your option) any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <http://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn {Function File} {} nextpow2 (@var{x})
## If @var{x} is a scalar, return the first integer @var{n} such that
## @iftex
## @tex
##  $2^n \ge |x|$.
## @end tex
## @end iftex
## @ifnottex
##  2^n >= abs (x).
## @end ifnottex
##
## If @var{x} is a vector, return @code{nextpow2 (length (@var{x}))}.
## @seealso{pow2, log2}
## @end deftypefn

## Author: KH <Kurt.Hornik@wu-wien.ac.at>
## Created: 7 October 1994
## Adapted-By: jwe

function n = nextpow2 (x)

  if (nargin != 1)
    print_usage ();
  endif

  if (! (isscalar (x) || isvector (x)))
    error ("nextpow2: x must be a scalar or a vector");
  endif

  t = length (x);
  if (t > 1)
    x = t;
  endif

  [f, n] = log2 (abs (x));
  if (f == 0.5)
    n = n - 1;
  endif

endfunction
