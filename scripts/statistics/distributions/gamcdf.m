## Copyright (C) 1995, 1996, 1997  Kurt Hornik
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
## @deftypefn {Function File} {} gamma_cdf (@var{x}, @var{a}, @var{b})
## For each element of @var{x}, compute the cumulative distribution
## function (CDF) at @var{x} of the Gamma distribution with parameters
## @var{a} and @var{b}.
## @end deftypefn

## Author: TT <Teresa.Twaroch@ci.tuwien.ac.at>
## Description: CDF of the Gamma distribution

function cdf = gamma_cdf (x, a, b)

  if (nargin != 3)
    usage ("gamma_cdf (x, a, b)");
  endif

  if (!isscalar (a) || !isscalar(b))
    [retval, x, a, b] = common_size (x, a, b);
    if (retval > 0)
      error ("gamma_cdf: x, a and b must be of common size or scalars");
    endif
  endif

  sz = size (x);
  cdf = zeros (sz);

  k = find (!(a > 0) | !(b > 0) | isnan (x));
  if (any (k))
    cdf (k) = NaN;
  endif

  k = find ((x > 0) & (a > 0) & (b > 0));
  if (any (k))
    if (isscalar (a) && isscalar(b))
      cdf (k) = gammainc (b * x(k), a);
    else
      cdf (k) = gammainc (b(k) .* x(k), a(k));
    endif
  endif

endfunction
