## Copyright (C) 2010 Jaroslav Hajek
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
## @deftypefn {Function File} {} powerset (@var{a})
## @deftypefnx {Function File} {} powerset (@var{a}, "rows")
##
## Return the cell array of all subsets of the set @var{a}.
##
## @end deftypefn
## @seealso{unique, union, setxor, setdiff, ismember}

function p = powerset (a, byrows_arg)

  byrows = false;

  if (nargin == 2)
    if (! strcmpi (byrows_arg, "rows"))
      error ("expecting third argument to be \"rows\"");
    elseif (iscell (a))
      warning ("setdiff: \"rows\" not valid for cell arrays");
    else
      byrows = true;
    endif
  elseif (nargin != 1)
    print_usage ();
  endif

  if (byrows)
    a = unique (a, byrows_arg);
    n = rows (a);
  else
    a = unique (a);
    n = numel (a);
  endif

  if (n == 0)
    p = [];
  else
    if (n > 32)
      error ("powerset: not implemented for more than 32 elements");
    endif

    ## Logical rep
    b = reshape (bitunpack (uint32 (0:2^n-1)), 32, 2^n)(1:n,:);
    ## Convert to indices and lengths.
    [i, k] = find (b);
    k = sum (b, 1);

    ## Index and split.
    if (byrows)
      p = mat2cell (a(i,:), k, columns (a));
    else
      if (rows (a) == 1)
        p = mat2cell (a(i), 1, k);
      else
        p = mat2cell (a(i), k, 1);
      endif
    endif
  endif

endfunction
