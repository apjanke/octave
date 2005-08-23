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
## @deftypefn {Function File} {} laplace_cdf (@var{x})
## For each element of @var{x}, compute the cumulative distribution
## function (CDF) at @var{x} of the Laplace distribution.
## @end deftypefn

## Author: KH <Kurt.Hornik@wu-wien.ac.at>
## Description: CDF of the Laplace distribution

function cdf = laplace_cdf (x)

  if (nargin != 1)
    usage ("laplace_cdf (x)");
  endif

  cdf = zeros (size (x));

  k = find (isnan (x));
  if (any (k))
    cdf(k) = NaN;
  endif

  k = find (x == Inf);
  if (any (k))
    cdf(k) = 1;
  endif

  k = find ((x > -Inf) & (x < Inf));
  if (any (k))
    cdf(k) = (1 + sign (x(k)) .* (1 - exp (- abs (x(k))))) / 2;
  endif

endfunction
