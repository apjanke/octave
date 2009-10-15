## Copyright (C) 1996, 1997, 1998, 1999, 2000, 2004, 2005, 2006, 2007,
##               2008, 2009 John W. Eaton
## Copyright (C) 2009 VZLU Prague
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
## @deftypefn {Function File} {} median (@var{x}, @var{dim})
## If @var{x} is a vector, compute the median value of the elements of
## @var{x}.  If the elements of @var{x} are sorted, the median is defined
## as
## @tex
## $$
## {\rm median} (x) =
##   \cases{x(\lceil N/2\rceil), & $N$ odd;\cr
##           (x(N/2)+x(N/2+1))/2, & $N$ even.}
## $$
## @end tex
## @ifnottex
##
## @example
## @group
##             x(ceil(N/2)),             N odd
## median(x) =
##             (x(N/2) + x((N/2)+1))/2,  N even
## @end group
## @end example
## @end ifnottex
## If @var{x} is a matrix, compute the median value for each
## column and return them in a row vector.  If the optional @var{dim}
## argument is given, operate along this dimension.
## @seealso{std, mean}
## @end deftypefn

## Author: jwe

function retval = median (a, dim)

  if (nargin != 1 && nargin != 2)
    print_usage ();
  endif
  if (nargin < 2)
    dim = [find(size (a) != 1, 1), 1](1); # First non-singleton dim.
  endif

  if (numel (a) > 0)
    n = size (a, dim);
    k = floor ((n+1) / 2);
    if (mod (n, 2) == 1)
      retval = nth_element (a, k, dim);
    else
      retval = mean (nth_element (a, k:k+1, dim), dim);
    endif
  else
    error ("median: invalid matrix argument");
  endif

endfunction

%!test
%! x = [1, 2, 3, 4, 5, 6];
%! x2 = x';
%! y = [1, 2, 3, 4, 5, 6, 7];
%! y2 = y';
%! 
%! assert((median (x) == median (x2) && median (x) == 3.5
%! && median (y) == median (y2) && median (y) == 4
%! && median ([x2, 2*x2]) == [3.5, 7]
%! && median ([y2, 3*y2]) == [4, 12]));

%!error median ();

%!error median (1, 2, 3);

