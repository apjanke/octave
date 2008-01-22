## Copyright (C) 1995, 1996, 1997, 1999, 2000, 2002, 2005, 2006, 2007
##               Kurt Hornik
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
## @deftypefn {Function File} {} vech (@var{x})
## Return the vector obtained by eliminating all supradiagonal elements of
## the square matrix @var{x} and stacking the result one column above the
## other.
## @end deftypefn

## See Magnus and Neudecker (1988), Matrix differential calculus with
## applications in statistics and econometrics.

## Author KH <Kurt.Hornik@wu-wien.ac.at>
## Created: 8 May 1995
## Adapted-By: jwe

function v = vech (x)

  if (nargin != 1)
    print_usage ();
  endif

  if (! issquare (x))
    error ("vech: x must be square");
  endif

  ## This should be quicker than having an inner `for' loop as well.
  ## Ideally, vech should be written in C++.
  n = rows (x);
  v = zeros ((n+1)*n/2, 1);
  count = 0;
  for j = 1 : n
    i = j : n;
    v (count + i) = x (i, j);
    count = count + n - j;
  endfor

endfunction

%!assert(all (vech ([1, 2, 3; 4, 5, 6; 7, 8, 9]) == [1; 4; 7; 5; 8; 9]));

%!error vech ();

%!error vech (1, 2);

