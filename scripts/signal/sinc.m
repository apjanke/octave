## Copyright (C) 1996, 1997 John W. Eaton
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
## @deftypefn {Function File} {} sinc (@var{x})
## Return
## @iftex
## @tex
## $ \sin (\pi x)/(\pi x)$.
## @end tex
## @end iftex
## @ifinfo
##  sin(pi*x)/(pi*x).
## @end ifinfo
## @end deftypefn

## Author: jwe ???

function result = sinc (x)

  ## We either need to set the do_fortran_indexing variable to "true"
  ## or use reshape to convert the input matrix to a vector, so that
  ## we can use find to determine the elements of x that equal zero.
  ## I prefer reshaping.

  [nr, nc] = size(x);

  nels = nr*nc;

  x = reshape(x,nels,1);

  ## Set result to all ones initially.
  result = ones(nels,1);

  ## Find non-zero elements in the input matrix.
  i = find(x);

  if (!isempty(i))
    result(i) = sin(pi*x(i))./(pi*x(i));
  endif

  result = reshape(result,nr,nc);

endfunction
