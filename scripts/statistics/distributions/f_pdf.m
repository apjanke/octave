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

## usage:  f_pdf (x, m, n)
##
## For each element of x, compute the probability density function (PDF)
## at x of the F distribution with m and n degrees of freedom.

## Author:  KH <Kurt.Hornik@ci.tuwien.ac.at>
## Description:  PDF of the F distribution

function pdf = f_pdf (x, m, n)
  
  if (nargin != 3)
    usage ("f_pdf (x, m, n).");
  endif
  
  [retval, x, m, n] = common_size (x, m, n);
  if (retval > 0)
    error ("f_pdf:  x, m and n must be of common size or scalar");
  endif
  
  [r, c] = size (x);
  s = r * c;
  x = reshape (x, 1, s);
  m = reshape (m, 1, s);
  n = reshape (n, 1, s);
  pdf = zeros (1, s);
  
  k = find (isnan (x) | !(m > 0) | !(n > 0));
  if any (k)
    pdf(k) = NaN * ones (1, length (k));
  endif
  
  k = find ((x > 0) & (x < Inf) & (m > 0) & (n > 0));
  if any (k)
    tmp = m(k) .* x(k) ./ n(k);
    pdf(k) = exp ((m(k) / 2 - 1) .* log (tmp) ...
	- ((m(k) + n(k)) / 2) .* log (1 + tmp)) ...
	.* (m(k) ./ n(k)) ./ beta (m(k) / 2, n(k) / 2);
  endif

  ## should we really only allow for positive integer m, n?
  k = find ((m != round (m)) | (n != round (n)));
  if any (k)
    fprintf (stderr, ...
	     "WARNING:  m and n should be positive integers\n");
    pdf(k) = NaN * ones (1, length (k));
  endif

  pdf = reshape (pdf, r, c);
  
endfunction
