## Copyright (C) 1993, 1994, 1995, 1996, 1997, 1998, 2004, 2006, 2007, 2009
##               Paul Kienzle
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
##
## Original version by Paul Kienzle distributed as free software in the
## public domain.

## -*- texinfo -*-
## @deftypefn {Function File} {} hadamard (@var{n})
## Construct a Hadamard matrix @var{Hn} of size @var{n}-by-@var{n}.  The 
## size @var{n} must be of the form @code{2 ^ @var{k} * @var{p}} in which
## @var{p} is one of 1, 12, 20 or 28.  The returned matrix is normalized,
## meaning @code{Hn(:,1) == 1} and @code{Hn(1,:) == 1}.
##
## Some of the properties of Hadamard matrices are:
##
## @itemize @bullet
## @item
## @code{kron (@var{Hm}, @var{Hn})} is a Hadamard matrix of size 
## @var{m}-by-@var{n}.
##
## @item
## @code{Hn * Hn' == @var{n} * eye (@var{n})}.
##
## @item
## The rows of @var{Hn} are orthogonal.
##
## @item
## @code{det (@var{A}) <= abs(det (@var{Hn}))} for all @var{A} with
## @code{abs (@var{A} (@var{i}, @var{j})) <= 1}.
##
## @item
## Multiply any row or column by -1 and still have a Hadamard matrix.
## @end itemize
##
## @end deftypefn

   
## Reference [1] contains a list of Hadamard matrices up to n=256.
## See code for h28 in hadamard.m for an example of how to extend
## this function for additional p.
##
## References:
## [1] A Library of Hadamard Matrices, N. J. A. Sloane 
##     http://www.research.att.com/~njas/hadamard/

function h = hadamard (n)
  
  if (nargin != 1)
    print_usage ();
  endif

  ## Find k if n = 2^k*p.
  k = 0;
  while (n > 1 && floor (n/2) == n/2)
    k++; 
    n = n/2; 
  endwhile
  
  ## Find base hadamard.
  ## Except for n=2^k, need a multiple of 4.
  if (n != 1)
    k -= 2; 
  endif

  ## Trigger error if not a multiple of 4.
  if (k < 0)
    n =- 1;
  endif

  switch (n)
    case 1
      h = 1;
    case 3
      h = h12 ();
    case 5
      h = h20 ();
    case 7
      h = hnormalize (h28 ());
    otherwise
      error ("hadamard: N must be 2^k*p, for p = 1, 12, 20 or 28");
  endswitch

  ## Build H(2^k*n) from kron(H(2^k),H(n)).
  h2 = [1,1;1,-1];
  while (true)
    if (floor (k/2) != k/2)
      h = kron (h2, h); 
    endif
    k = floor (k/2);
    if (k == 0) 
      break; 
    endif
    h2 = kron (h2, h2);
  endwhile
endfunction

function h = h12 ()
  tu = [-1,+1,-1,+1,+1,+1,-1,-1,-1,+1,-1];
  tl = [-1,-1,+1,-1,-1,-1,+1,+1,+1,-1,+1];
  ## Note: assert (tu(2:end), tl(end:-1:2)).
  h = ones (12);
  h(2:end,2:end) = toeplitz (tu, tl);
endfunction


function h = h20 ()
  tu = [+1,-1,-1,+1,+1,+1,+1,-1,+1,-1,+1,-1,-1,-1,-1,+1,+1,-1,-1];
  tl = [+1,-1,-1,+1,+1,-1,-1,-1,-1,+1,-1,+1,-1,+1,+1,+1,+1,-1,-1];
  ## Note: assert (tu(2:end), tl(end:-1:2)).
  h = ones (20);
  h(2:end,2:end) = fliplr (toeplitz (tu, tl));
endfunction

function h = hnormalize (h)
  ## Make sure each row and column starts with +1.
  h(h(:,1)==-1,:) *= -1;
  h(:,h(1,:)==-1) *= -1;
endfunction

function h = h28 ()
  ## Williamson matrix construction from
  ## http://www.research.att.com/~njas/hadamard/had.28.will.txt
  s = ["+------++----++-+--+-+--++--";
       "-+-----+++-----+-+--+-+--++-";
       "--+-----+++---+-+-+----+--++";
       "---+-----+++---+-+-+-+--+--+";
       "----+-----+++---+-+-+++--+--";
       "-----+-----++++--+-+--++--+-";
       "------++----++-+--+-+--++--+";
       "--++++-+-------++--+++-+--+-";
       "---++++-+-----+-++--+-+-+--+";
       "+---+++--+----++-++--+-+-+--";
       "++---++---+----++-++--+-+-+-";
       "+++---+----+----++-++--+-+-+";
       "++++--------+-+--++-++--+-+-";
       "-++++--------+++--++--+--+-+";
       "-+-++-++--++--+--------++++-";
       "+-+-++--+--++--+--------++++";
       "-+-+-++--+--++--+----+---+++";
       "+-+-+-++--+--+---+---++---++";
       "++-+-+-++--+------+--+++---+";
       "-++-+-+-++--+------+-++++---";
       "+-++-+---++--+------+-++++--";
       "-++--++-+-++-+++----++------";
       "+-++--++-+-++-+++-----+-----";
       "++-++---+-+-++-+++-----+----";
       "-++-++-+-+-+-+--+++-----+---";
       "--++-++++-+-+----+++-----+--";
       "+--++-+-++-+-+----+++-----+-";
       "++--++-+-++-+-+----++------+"];
  ## Without this, the assignment of -1 will not work properly
  ## (compatibility forces LHS(idx) = ANY_VAL to keep the LHS logical
  ## instead of widening to a type that can represent ANY_VAL).
  h = double (s == "+");
  h(!h) = -1;
endfunction

%!assert(hadamard(1),1)
%!assert(hadamard(2),[1,1;1,-1])
%!test
%!  for n=[1,2,4,8,12,24,48,20,28,2^9]
%!    h=hadamard(n); assert(norm(h*h'-n*eye(n)),0);
%!  end
