## Copyright (C) 1995, 1996, 1997  Kurt Hornik
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

## -*- texinfo -*-
## @deftypefn {Function File} {} var (@var{x})
## For vector arguments, return the (real) variance of the values.
## For matrix arguments, return a row vector contaning the variance for
## each column.
## @end deftypefn

## Author: KH <Kurt.Hornik@ci.tuwien.ac.at>
## Description: Compute variance

function y = var(x)

  if (nargin != 1)
    usage ("var (x)");
  endif

  [nr, nc] = size (x);
  if (nr == 0 || nc == 0)
    error ("var: x must not be empty");
  elseif ((nr == 1) && (nc == 1))
    y = 0;
  elseif ((nr == 1) || (nc == 1))
    n = length (x);
    y = (sumsq (x) - sum(x)^2 / n) / (n - 1);
  else
    y = (sumsq (x) - sum(x).^2 / nr) / (nr - 1);
  endif

endfunction
