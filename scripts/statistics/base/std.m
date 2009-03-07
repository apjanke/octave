## Copyright (C) 1996, 1997, 1998, 1999, 2000, 2004, 2005, 2006, 2007, 2008,
##               2009 John W. Eaton
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
## @deftypefn {Function File} {} std (@var{x})
## @deftypefnx {Function File} {} std (@var{x}, @var{opt})
## @deftypefnx {Function File} {} std (@var{x}, @var{opt}, @var{dim})
## If @var{x} is a vector, compute the standard deviation of the elements
## of @var{x}.
## @iftex
## @tex
## $$
## {\rm std} (x) = \sigma (x) = \sqrt{{\sum_{i=1}^N (x_i - \bar{x})^2 \over N - 1}}
## $$
## where $\bar{x}$ is the mean value of $x$.
## @end tex
## @end iftex
## @ifnottex
##
## @example
## @group
## std (x) = sqrt (sumsq (x - mean (x)) / (n - 1))
## @end group
## @end example
## @end ifnottex
## If @var{x} is a matrix, compute the standard deviation for
## each column and return them in a row vector.
##
## The argument @var{opt} determines the type of normalization to use. Valid values
## are
##
## @table @asis 
## @item 0:
##   normalizes with @math{N-1}, provides the square root of best unbiased estimator of 
##   the variance [default]
## @item 1:
##   normalizes with @math{N}, this provides the square root of the second moment around 
##   the mean
## @end table
##
## The third argument @var{dim} determines the dimension along which the standard
## deviation is calculated.
## @seealso{mean, median}
## @end deftypefn

## Author: jwe

function retval = std (a, opt, dim)

  if (nargin < 1 || nargin > 3)
    print_usage ();
  endif
  if nargin < 3
    dim = find (size (a) > 1, 1);
    if (isempty (dim))
      dim = 1;
    endif
  endif
  if (nargin < 2 || isempty (opt))
    opt = 0;
  endif

  sz = size(a);
  if (sz (dim) == 1)
    retval = zeros(sz);
  elseif (numel (a) > 0)
    rng = ones (1, length (sz));
    rng (dim) = sz (dim);
    if (opt == 0)
      retval = sqrt (sumsq (a - repmat(mean (a, dim), rng), dim) / (sz(dim) - 1));
    else
      retval = sqrt (sumsq (a - repmat(mean (a, dim), rng), dim) / sz(dim));
    endif
  else
    error ("std: invalid matrix argument");
  endif

endfunction

%!test
%! x = ones (10, 2);
%! y = [1, 3];
%! assert(std (x) == [0, 0] && abs (std (y) - sqrt (2)) < sqrt (eps));

%!error std ();

%!error std (1, 2, 3, 4);

