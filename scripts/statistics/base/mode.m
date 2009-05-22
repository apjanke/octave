## Copyright (C) 2007, 2008, 2009 David Bateman
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
## @deftypefn {Function File} {[@var{m}, @var{f}, @var{c}] =} mode (@var{x}, @var{dim})
## Count the most frequently appearing value.  @code{mode} counts the 
## frequency along the first non-singleton dimension and if two or more
## values have the same frequency returns the smallest of the two in
## @var{m}.  The dimension along which to count can be specified by the
## @var{dim} parameter.
##
## The variable @var{f} counts the frequency of each of the most frequently 
## occurring elements.  The cell array @var{c} contains all of the elements
## with the maximum frequency .
## @end deftypefn

function [m, f, c] = mode (x, dim)

  if (nargin < 1 || nargin > 2)
    print_usage ();
  endif

  nd = ndims (x);
  sz = size (x);

  if (nargin != 2)
    ## Find the first non-singleton dimension.
    dim  = 1;
    while (dim < nd + 1 && sz(dim) == 1)
      dim = dim + 1;
    endwhile
    if (dim > nd)
      dim = 1;
    endif
  else
    if (! (isscalar (dim) && dim == round (dim))
        && dim > 0
        && dim < (nd + 1))
      error ("mode: dim must be an integer and valid dimension");
    endif
  endif

  sz2 = sz;
  sz2 (dim) = 1;
  sz3 = ones (1, nd);
  sz3 (dim) = sz (dim);

  if (issparse (x))
    t2 = sparse (sz(1), sz(2));
  else
    t2 = zeros (sz);
  endif

  if (dim != 1)
    perm = [dim, 1:dim-1, dim+1:nd];
    t2 = permute (t2, perm);
  endif

  xs = sort (x, dim);
  t = cat (dim, true (sz2), diff (xs, 1, dim) != 0);

  if (dim != 1)
    t2 (permute (t != 0, perm)) = diff ([find(permute (t, perm))(:); prod(sz)+1]);
    f = max (ipermute (t2, perm), [], dim);
    xs = permute (xs, perm);
  else
    t2 (t) = diff ([find(t)(:); prod(sz)+1]);
    f = max (t2, [], dim);
  endif

  c = cell (sz2);
  if (issparse (x))
    m = sparse (sz2(1), sz2(2));
  else
    m = zeros (sz2);
  endif
  for i = 1 : prod (sz2)
    c{i} = xs (t2 (:, i) == f(i), i);
    m (i) = c{i}(1);
  endfor
endfunction

%!test
%! [m, f, c] = mode (toeplitz (1:5));
%! assert (m, [1,2,2,2,1]);
%! assert (f, [1,2,2,2,1]);
%! assert (c, {[1;2;3;4;5],[2],[2;3],[2],[1;2;3;4;5]});
%!test
%! [m, f, c] = mode (toeplitz (1:5), 2);
%! assert (m, [1;2;2;2;1]);
%! assert (f, [1;2;2;2;1]);
%! assert (c, {[1;2;3;4;5];[2];[2;3];[2];[1;2;3;4;5]});
%!test
%! a = sprandn (32, 32, 0.05);
%! [m, f, c] = mode (a);
%! [m2, f2, c2] = mode (full (a));
%! assert (m, sparse (m2));
%! assert (f, sparse (f2));
%! assert (c, cellfun (@(x) sparse (0), c2, 'UniformOutput', false));

%!assert(mode([2,3,1,2,3,4],1),[2,3,1,2,3,4])
%!assert(mode([2,3,1,2,3,4],2),2)
%!assert(mode([2,3,1,2,3,4]),2)

%!assert(mode([2;3;1;2;3;4],1),2)
%!assert(mode([2;3;1;2;3;4],2),[2;3;1;2;3;4])
%!assert(mode([2;3;1;2;3;4]),2)

%!shared x
%! x(:,:,1) = toeplitz (1:3);
%! x(:,:,2) = circshift (toeplitz (1:3), 1);
%! x(:,:,3) = circshift (toeplitz (1:3), 2);
%!test
%! [m, f, c] = mode (x, 1);
%! assert (reshape (m, [3, 3]), [1 1 1; 2 2 2; 1 1 1])
%! assert (reshape (f, [3, 3]), [1 1 1; 2 2 2; 1 1 1])
%! c = reshape (c, [3, 3]);
%! assert (c{1}, [1; 2; 3])
%! assert (c{2}, 2)
%! assert (c{3}, [1; 2; 3])
%!test
%! [m, f, c] = mode (x, 2);
%! assert (reshape (m, [3, 3]), [1 1 2; 2 1 1; 1 2 1])
%! assert (reshape (f, [3, 3]), [1 1 2; 2 1 1; 1 2 1])
%! c = reshape (c, [3, 3]);
%! assert (c{1}, [1; 2; 3])
%! assert (c{2}, 2)
%! assert (c{3}, [1; 2; 3])
%!test
%! [m, f, c] = mode (x, 3);
%! assert (reshape (m, [3, 3]), [1 2 1; 1 2 1; 1 2 1])
%! assert (reshape (f, [3, 3]), [1 2 1; 1 2 1; 1 2 1])
%! c = reshape (c, [3, 3]);
%! assert (c{1}, [1; 2; 3])
%! assert (c{2}, [1; 2; 3])
%! assert (c{3}, [1; 2; 3])
