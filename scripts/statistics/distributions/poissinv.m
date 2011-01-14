## Copyright (C) 1995-2011 Kurt Hornik
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
## @deftypefn {Function File} {} poissinv (@var{x}, @var{lambda})
## For each component of @var{x}, compute the quantile (the inverse of
## the CDF) at @var{x} of the Poisson distribution with parameter
## @var{lambda}.
## @end deftypefn

## Author: KH <Kurt.Hornik@wu-wien.ac.at>
## Description: Quantile function of the Poisson distribution

function inv = poissinv (x, lambda)

  if (nargin != 2)
    print_usage ();
  endif

  if (!isscalar (lambda))
    [retval, x, lambda] = common_size (x, lambda);
    if (retval > 0)
      error ("poissinv: X and LAMBDA must be of common size or scalar");
    endif
  endif

  inv = zeros (size (x));

  k = find ((x < 0) | (x > 1) | isnan (x) | !(lambda > 0));
  if (any (k))
    inv(k) = NaN;
  endif

  k = find ((x == 1) & (lambda > 0));
  if (any (k))
    inv(k) = Inf;
  endif

  k = find ((x > 0) & (x < 1) & (lambda > 0));
  if (any (k))
    if (isscalar (lambda))
      cdf = exp (-lambda) * ones (size (k));
    else
      cdf = exp (-lambda(k));
    endif
    while (1)
      m = find (cdf < x(k));
      if (any (m))
        inv(k(m)) = inv(k(m)) + 1;
        if (isscalar (lambda))
          cdf(m) = cdf(m) + poisspdf (inv(k(m)), lambda);
        else
          cdf(m) = cdf(m) + poisspdf (inv(k(m)), lambda(k(m)));
        endif
      else
        break;
      endif
    endwhile
  endif

endfunction
