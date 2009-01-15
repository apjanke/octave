## Copyright (C) 1994, 1996, 1997, 1999, 2000, 2004, 2005, 2007
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
## @deftypefn {Function File} {} sinc (@var{x})
## Return
## @iftex
## @tex
## $ \sin (\pi x)/(\pi x)$.
## @end tex
## @end iftex
## @ifnottex
##  sin(pi*x)/(pi*x).
## @end ifnottex
## @end deftypefn

## Author: jwe ???

function result = sinc (x)

  if (nargin != 1)
    print_usage ();
  endif

  result = ones (size (x));

  i = (x != 0);

  if (any (i(:)))
    t = pi * x(i);
    result(i) = sin (t) ./ t;
  endif

endfunction
