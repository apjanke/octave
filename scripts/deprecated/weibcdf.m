## Copyright (C) 2006, 2007 John W. Eaton
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
## @deftypefn {Function File} {} weibcdf (@var{x}, @var{scale}, @var{shape})
## Compute the cumulative distribution function (CDF) at @var{x} of the
## Weibull distribution with shape parameter @var{scale} and scale
## parameter @var{shape}, which is
##
## @example
## 1 - exp(-(x/shape)^scale)
## @end example
##
## @noindent
## for @var{x} >= 0.
## @end deftypefn

function cdf = weibcdf (varargin)

  cdf = wblcdf (varargin{:});

endfunction
