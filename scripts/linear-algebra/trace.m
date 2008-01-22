## Copyright (C) 1993, 1994, 1995, 1996, 1997, 1999, 2005, 2006, 2007
##               John W. Eaton
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
## @deftypefn {Function File} {} trace (@var{a})
## Compute the trace of @var{a}, @code{sum (diag (@var{a}))}.
## @end deftypefn

## Author: jwe

function y = trace (x)

  if (nargin != 1)
    print_usage ();
  endif

  [nr, nc] = size (x);
  if (nr == 1 || nc == 1)
    y = x(1);
  else
    y = sum (diag (x));
  endif

endfunction

%!assert(trace ([1, 2; 3, 4]) == 5);

%!assert(trace ([1, 2; 3, 4; 5, 6]) == 5);

%!assert(trace ([1, 3, 5; 2, 4, 6]) == 5);

%!error trace ();

%!error trace (1, 2);

