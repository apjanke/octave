## Copyright (C) 2007  David Bateman
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
## @deftypefn {Loadable Function} {@var{idx} =} dsearch (@var{x}, @var{y}, @var{tri}, @var{xi}, @var{yi})
## @deftypefnx {Loadable Function} {@var{idx} =} dsearch (@var{x}, @var{y}, @var{tri}, @var{xi}, @var{yi}, @var{s})
## Returns the index @var{idx} or the closest point in @code{@var{x}, @var{y})}
## to the elements @code{[@var{xi}(:), @var{yi}(:)]}. The variable @var{s} is
## accepted but ignored for compatibility.
## @seealso{dsearchn, tsearch}
## @end deftypefn

function idx = dsearch (x, y, t, xi, yi, s)
  if (nargin < 5 || nargin > 6)
    print_usage();
  endif
  idx = __dsearchn__ ([x(:),y(:)], [xi(:), yi(:)]);
endfunction

%!shared x, y, tri
%! x = [-1;-1;1];
%! y = [-1;1;-1];
%! tri = [1,2,3]; 
%!assert (dsearch(x,y,tri,1,1/3), 3);
%!assert (dsearch(x,y,tri,1/3,1), 2);
