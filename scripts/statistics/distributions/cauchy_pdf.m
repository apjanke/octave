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
## Software Foundation, 59 Temple Place - Suite 330, Boston, MA
## 02111-1307, USA.

## -*- texinfo -*-
## @deftypefn {Function File} {} cauchy_pdf (@var{x}, @var{lambda}, @var{sigma})
## For each element of @var{x}, compute the probability density function
## (PDF) at @var{x} of the Cauchy distribution with location parameter
## @var{lambda} and scale parameter @var{sigma} > 0.  Default values are
## @var{lambda} = 0, @var{sigma} = 1. 
## @end deftypefn

## Author: KH <Kurt.Hornik@ci.tuwien.ac.at>
## Description: PDF of the Cauchy distribution

function pdf = cauchy_pdf (x, location, scale)

  if (! (nargin == 1 || nargin == 3))
    usage ("cauchy_pdf (x, lambda, sigma)");
  endif

  if (nargin == 1)
    location = 0;
    scale = 1;
  endif

  [retval, x, location, scale] = common_size (x, location, scale);
  if (retval > 0)
    error ("cauchy_pdf: x, lambda and sigma must be of common size or scalar");
  endif

  [r, c] = size (x);
  s = r * c;
  x = reshape (x, 1, s);
  location = reshape (location, 1, s);
  scale = reshape (scale, 1, s);

  pdf = NaN * ones (1, s);

  k = find ((x > -Inf) & (x < Inf) & (location > -Inf) &
            (location < Inf) & (scale > 0) & (scale < Inf));
  if (any (k))
    pdf(k) = ((1 ./ (1 + ((x(k) - location(k)) ./ scale(k)) .^ 2))
	      / pi ./ scale(k));
  endif

  pdf = reshape (pdf, r, c);

endfunction
