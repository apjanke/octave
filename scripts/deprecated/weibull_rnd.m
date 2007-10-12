## Copyright (C) 2005, 2006, 2007 John W. Eaton
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
## @deftypefn {Function File} {} weibull_rnd (@var{shape}, @var{scale}, @var{r}, @var{c})
## @deftypefnx {Function File} {} weibull_rnd (@var{shape}, @var{scale}, @var{sz})
## Return an @var{r} by @var{c} matrix of random samples from the
## Weibull distribution with parameters @var{scale} and @var{shape}
## which must be scalar or of size @var{r} by @var{c}. Or if @var{sz}
## is a vector return a matrix of size @var{sz}.
##
## If @var{r} and @var{c} are omitted, the size of the result matrix is
## the common size of @var{alpha} and @var{sigma}.
## @end deftypefn

function rnd = weibull_rnd (varargin)

  if (nargin > 1)
    tmp = varargin{2};
    varargin{2} = varargin{1};
    varargin{1} = tmp;
  endif

  rnd = wblrnd (varargin{:});

endfunction

