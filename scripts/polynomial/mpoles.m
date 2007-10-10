## Copyright (C) 2007 Ben Abbott
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
## @deftypefn {Function File} {[@var{multp}, @var{indx}] =} mpoles (@var{p})
## @deftypefnx {Function File} {[@var{multp}, @var{indx}] =} mpoles (@var{p}, @var{tol})
## @deftypefnx {Function File} {[@var{multp}, @var{indx}] =} mpoles (@var{p}, @var{tol}, @var{reorder})
## Identifiy unique poles in @var{p} and associates their multiplicity,
## ordering them from largest to smallest.
## 
## If the relative difference of the poles is less than @var{tol}, then
## they are considered to be multiples.  The default value for @var{tol}
## is 0.001.
##
## If the optional parameter @var{reorder} is zero, poles are not sorted.
##
## The value @var{multp} is a vector specifying the multiplicity of the
## poles.  @var{multp}(:) refers to mulitplicity of @var{p}(@var{indx}(:)).
##
## For example,
##
## @example
## @group
## p = [2 3 1 1 2];
## [m, n] = mpoles(p);
##   @result{} m = [1; 1; 2; 1; 2]
##   @result{} n = [2; 5; 1; 4; 3]
##   @result{} p(n) = [3, 2, 2, 1, 1]
## @end group
## @end example
##
## @seealso{poly, roots, conv, deconv, polyval, polyderiv, polyinteg, residue}
## @end deftypefn

## Author: Ben Abbott <bpabbott@mac.com>
## Created: Sept 30, 2007

function [multp, indx] = mpoles (p, tol, reorder)

  if (nargin < 1 || nargin > 3)
    print_usage ();
  endif

   if (nargin < 2 || isempty (tol))
     tol = 0.001;
   endif

   if (nargin < 3 || isempty (reorder))
     reorder = true;
   endif

  Np = numel (p);

  ## Force the poles to be a column vector.

  p = p(:);

  ## Sort the poles according to their magnitidues, largest first.

  if (reorder)
    ## Sort with smallest magnitude first.
    [p, ordr] = sort (p);
    ## Reverse order, largest maginitude first.
    n = Np:-1:1;
    p = p(n);
    ordr = ordr(n);
  else
    ordr = 1:Np;
  endif

  ## Find pole multiplicty by comparing the relative differnce in the
  ## poles.

  multp = zeros (Np, 1);
  indx = [];
  n = find (multp == 0, 1);
  while (n)
    dp = abs (p-p(n));
    if (p(n) == 0.0)
      p0 = mean (abs (p(find (abs (p) > 0))));
      if (isempty (p0))
        p0 = 1;
      end
    else
      p0 = abs (p(n));
    endif
    k = find (dp < tol * p0);
    m = 1:numel (k);
    multp(k) = m;
    indx = [indx; k];
    n = find (multp == 0, 1);
  endwhile
  multp = multp(indx);
  indx = ordr(indx);

endfunction
