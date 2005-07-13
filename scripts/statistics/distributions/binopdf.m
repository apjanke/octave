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
## @deftypefn {Function File} {} binopdf (@var{x}, @var{n}, @var{p})
## For each element of @var{x}, compute the probability density function
## (PDF) at @var{x} of the binomial distribution with parameters @var{n}
## and @var{p}.
## @end deftypefn

## Author: KH <Kurt.Hornik@ci.tuwien.ac.at>
## Description: PDF of the binomial distribution

function pdf = binopdf (x, n, p)

  if (nargin != 3)
    usage ("binopdf (x, n, p)");
  endif

  if (! isscalar (n) || ! isscalar (p))
    [retval, x, n, p] = common_size (x, n, p);
    if (retval > 0)
      error ("binopdf: x, n and p must be of common size or scalar");
    endif
  endif

  k = ((x >= 0) & (x <= n)
       & (x == round (x)) & (n == round (n))
       & (p >= 0) & (p <= 1));

  pdf = zeros (size (x));
  pdf(! k) = NaN;
  if (any (k(:)))
    x = x(k);
    if (! isscalar (n))
      n = n(k);
    endif
    if (! isscalar (p))
      p = p(k);
    endif
    z = gammaln(n+1) - gammaln(x+1) - gammaln(n-x+1) + x.*log(p) + (n-x).*log(1-p);
    pdf(k) = exp (z);
  endif

endfunction
