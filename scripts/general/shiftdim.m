## Copyright (C) 2004 John Eaton, David Bateman
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
## @deftypefn {Function File} {@var{y}} = shiftdim (@var{x}, @var{n})
## @deftypefnx {Function File} {[@var{y}, @var{ns}]} = shiftdim (@var{x})
## Shifts the dimension of @var{x} by @var{n}, where @var{n} must be
## an integer scalar. When @var{n} is negative, the dimensions of
## @var{x} are shifted to the left, with the leading dimensions
## circulated to the end. If @var{n} is positive, then the dimensions
## of @var{x} are shifted to the right, with the @var{n} singleton
## dimensions added.
##
## Called with a single argument, @code{shiftdim}, removes the leading
## singleton dimensions, returning the number of dimensions removed
## in the second output argument @var{ns}.
##
## For example 
##
## @example
## @group
## x = ones (1, 2, 3);
## size (shiftdim (x, -1))
##      @result{} [2, 3, 1]
## size (shiftdim (x, 1))
##      @result{} [1, 1, 2, 3]
## [b, ns] = shiftdim (x);
##      @result{} b =  [1, 1, 1; 1, 1, 1]
##      @result{} ns = 1
## @end group
## @end example
## @end deftypefn
##
## @seealso {reshape, permute, ipermute, circshift, squeeze}

function [y, ns]  = shiftdim (x, n)

  if (nargin < 1 || nargin > 2)
    usage ("shiftdim (x, n) or [b, ns] = shiftdim (x)");
  endif

  nd = ndims (x);
  orig_dims = size (x);

  if (nargin == 1)
    ## Find the first singleton dimension.
    n = 0;
    while (n < nd && orig_dims(n+1) == 1)
      n++;
    endwhile
  endif

  if (! isscalar (n) || floor (n) != n)
    error ("shiftdim: n must be a scalar integer");
  endif

  if (n >= nd)
    n = rem (n, nd);
  endif

  if (n < 0)
    singleton_dims = ones (1, -n);
    y = reshape (x, [singleton_dims, orig_dims]);
  elseif (n > 0)
    y = reshape (x, [orig_dims(n+1:nd), orig_dims(1:n)]);
  else
    y = x;
  endif

  ns = n;

endfunction
