# Copyright (C) 1994 John W. Eaton
#
# This file is part of Octave.
#
# Octave is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any
# later version.
#
# Octave is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with Octave; see the file COPYING.  If not, write to the Free
# Software Foundation, 59 Temple Place, Suite 330, Boston, MA 02111 USA.

## -*- texinfo -*-
## @deftypefn {Function File } { } pinv ( @var{X}@{,@var{tol}@} ) 
## Returns the pseudoinverse of X; singular values less than tol are ignored.
##  
## If the second arguement is ommited , it is assummed that
## @example
##   tol = max (size (X)) * sigma_max (X) * eps,
## @end example
## where sigma_max(X) is the maximal singular value of X.  
## @end deftypefn

function retval = pinv (X, tol)
# Written by Kurt Hornik (hornik@neuro.tuwien.ac.at) March 1993.
# Dept of Probability Theory and Statistics TU Wien, Austria.

  if (nargin < 1 || nargin > 2)
    error ("usage: pinv (X [, tol])");
  endif

  [U, S, V] = svd(X);
  s = diag(S);

  if (nargin == 1)
    tol = max (size (X)) * s (1) * eps;
  endif

  r = sum (s > tol);
  if (r == 0)
    retval = zeros (X');
  else
    D = diag (ones (r, 1) ./ s (1:r));
    retval = V (:, 1:r) * D * U (:, 1:r)';
  endif

endfunction
