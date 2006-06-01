## Copyright (C) 2002 Rolf Fabian <fabian@tu-cottbus.de>
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, write to the Free
## Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
## 02110-1301, USA.

## -*- texinfo -*-
## @deftypefn {Function File} {@var{s} =} mat2str (@var{x}, @var{n})
## @deftypefnx {Function File} {@var{s} =} mat2str (@dots{}, 'class')
##
## Format real/complex numerial matrices as strings. This function
## returns values that are suitable for the use of the @code{eval}
## function.
##
## The precision of the values is given by @var{n}. If @var{n} is a
## scalar then both real and imaginary parts of the matrix are printed
## to the same precision. Otherwise @code{@var{n} (1)} defines the
## precision of the real part and @code{@var{n} (2)} defines the
## precision of the imaginary part. The default for @var{n} is 17.
##
## If the argument 'class' is given, then the class of @var{x} is
## included in the string in such a way that the eval will result in the
## construction of a matrix of the same class.
##
## @example
## @group
##    mat2str( [ -1/3 + i/7; 1/3 - i/7 ], [4 2] )
##    @result{} '[-0.3333+0.14i;0.3333-0.14i]'
##    mat2str( [ -1/3 +i/7; 1/3 -i/7 ], [4 2] )
##    @result{} '[-0.3333+0i,0+0.14i;0.3333+0i,-0-0.14i]'
##    mat2str( int16([1 -1]), 'class')
##    @result{} 'int16([1,-1])'
## @end group
## @end example
##
## @seealso{sprintf, int2str}
## @end deftypefn

function s = mat2str (x, n, cls)

  if (nargin < 2 || isempty (n))
    ## Default precision
    n = 17;
  endif

  if (nargin < 3)
    if (ischar (n))
      cls = n;
      n = 17;
    else
      cls = "";
    endif
  endif

  if (nargin < 1 || nargin > 3 || ! isnumeric (x))
    print_usage ();
  endif
  
  if (ndims (x) > 2)
    error ("mat2str: X must be two dimensional");
  endif

  x_is_complex = is_complex (x);

  if (! x_is_complex)
    fmt = sprintf ("%%.%dg", n(1));
  else
    if (length (n) == 1 )
      n = [n, n];
    endif
    fmt = sprintf ("%%.%dg%%+.%dgi", n(1), n(2));
  endif

  nel = numel (x);

  if (nel == 0)
    ## Empty, only print brackets
    s = "[]";
  elseif (nel == 1)
    ## Scalar X, don't print brackets
    if (! x_is_complex)
      s = sprintf (fmt, x);
    else
      s = sprintf (fmt, real (x), imag (x));
    endif
  else
    ## Non-scalar X, print brackets
    fmt = [fmt, ","];
    if (! x_is_complex)
      s = sprintf (fmt, x.');
    else
      x = x.';
      s = sprintf (fmt, [real(x(:))'; imag(x(:))']);
    endif

    s = ["[", s];
    s(end) = "]";
    ind = find (s == ",");
    s(ind(nc:nc:end)) = ";";
  endif

  if (strcmp ("class", cls))
    s = [class(x), "(", s, ")"]
  endif
endfunction
