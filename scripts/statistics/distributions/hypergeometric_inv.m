## Copyright (C) 1997  Kurt Hornik
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this file.  If not, write to the Free Software Foundation,
## 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

## For each element of x, compute the quantile at x of the hypergeometric
## distribution with parameters m, t, and n.
##
## The parameters m, t, and n must positive integers with m and n not
## greater than t.

## Author:  KH <Kurt.Hornik@ci.tuwien.ac.at>
## Description:  Random deviates from the hypergeometric distribution

function inv = hypergeometric_inv (x, m, t, n)

  if (nargin != 4)
    usage ("hypergeometric_inv (x, m, t, n)");
  endif

  if ((m < 0) | (t < 0) | (n <= 0) | (m != round (m)) |
      (t != round (t)) | (n != round (n)) | (m > t) | (n > t))
    inv = NaN * ones (size (x))
  else
    inv = discrete_inv (x, 0 : n, hypergeometric_pdf (0 : n, m, t, n));
  endif

endfunction
