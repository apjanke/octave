## Copyright (C) 2000 Paul Kienzle
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
## @deftypefn {Function File} {@var{p} =} factor (@var{q})
## @deftypefnx {Function File} {[@var{p}, @var{n}] =} factor (@var{q})
##
## Return prime factorization of @var{q}. That is @code{prod (@var{p})
## == @var{q}}. If @code{@var{q} == 1}, returns 1. 
##
## With two output arguments, returns the unique primes @var{p} and
## their multiplicities. That is @code{prod (@var{p} .^ @var{n}) ==
## @var{q}}.
## 
## @end deftypefn

## Author: Paul Kienzle

## 2002-01-28 Paul Kienzle
## * remove recursion; only check existing primes for multiplicity > 1
## * return multiplicity as suggested by Dirk Laurie
## * add error handling

function [x, m] = factor (n)

  if (nargin < 1)
    print_usage ();
  endif

  if (! isscalar (n) || n != fix (n))
    error ("factor: n must be a scalar integer");
  endif

  ## special case of no primes less than sqrt(n)
  if (n < 4)
    x = n;
    m = 1;
    return;
  endif 

  x = [];
  ## There is at most one prime greater than sqrt(n), and if it exists,
  ## it has multiplicity 1, so no need to consider any factors greater
  ## than sqrt(n) directly. [If there were two factors p1, p2 > sqrt(n),
  ## then n >= p1*p2 > sqrt(n)*sqrt(n) == n. Contradiction.]
  p = primes (sqrt (n));
  while (n > 1)
    ## find prime factors in remaining n
    q = n ./ p;
    p = p (q == fix (q));
    if (isempty (p))
      p = n;  # can't be reduced further, so n must itself be a prime.
    endif
    x = [x, p];
    ## reduce n
    n = n / prod (p);
  endwhile
  x = sort (x);

  ## determine muliplicity
  if (nargout > 1)
    idx = find ([0, x] != [x, 0]);
    x = x(idx(1:length(idx)-1));
    m = diff (idx);
  endif

endfunction

## test:
##   assert(factor(1),1);
##   for i=2:20
##      p = factor(i);
##      assert(prod(p),i);
##      assert(all(isprime(p)));
##      [p,n] = factor(i);
##      assert(prod(p.^n),i);
##      assert(all([0,p]!=[p,0]));
##   end
