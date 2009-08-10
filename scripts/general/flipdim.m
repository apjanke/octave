## Copyright (C) 2004, 2005, 2006, 2007 David Bateman
## Copyright (C) 2009 VZLU Prague
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
## @deftypefn {Function File} {} flipdim (@var{x}, @var{dim})
## Return a copy of @var{x} flipped about the dimension @var{dim}.
## For example
##
## @example
## @group
## flipdim ([1, 2; 3, 4], 2)
##      @result{}  2  1
##          4  3
## @end group
## @end example
## @seealso{fliplr, flipud, rot90, rotdim}
## @end deftypefn

## Author: David Bateman, Jaroslav Hajek

function y = flipdim (x, dim)

  if (nargin != 1 && nargin != 2)
    print_usage ();
  endif

  nd = ndims (x);
  if (nargin == 1)
    ## Find the first non-singleton dimension.
    dim = find (size (x) != 1, 1);
    if (isempty (dim))
      dim = 1;
    endif
  else
    if (! (isscalar (dim) && isindex (dim, nd)))
      error ("flipdim: dim must be an integer and valid dimension");
    endif
  endif

  idx(1:nd) = {':'};
  idx{dim} = size (x, dim):-1:1;
  y = x(idx{:});

endfunction
