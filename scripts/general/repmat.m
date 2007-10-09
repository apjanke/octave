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
## @deftypefn {Function File} {} repmat (@var{A}, @var{m}, @var{n})
## @deftypefnx {Function File} {} repmat (@var{A}, [@var{m} @var{n}])
## @deftypefnx {Function File} {} repmat (@var{A}, [@var{m} @var{n} @var{p} ...])
## Form a block matrix of size @var{m} by @var{n}, with a copy of matrix
## @var{A} as each element.  If @var{n} is not specified, form an 
## @var{m} by @var{m} block matrix.
## @end deftypefn

## Author: Paul Kienzle <pkienzle@kienzle.powernet.co.uk>
## Created: July 2000

function x = repmat (a, m, n)

  if (nargin < 2 || nargin > 3)
    print_usage ();
  endif

  if (nargin == 3)
    if (! (isscalar (m) && isscalar (n)))
      error ("repmat: with 3 arguments m and n must be scalar");
    endif
    idx = [m, n];
  else 
    if (isscalar (m))
      idx = [m, m];
      n = m;
    elseif (isvector (m) && length (m) > 1)
      # Ensure that we have a row vector
      idx = m(:).';
    else
      error ("repmat: invalid dimensional argument");
    endif
  endif

  if (numel (a) == 1)
    if (ischar (a))
      x = char (toascii (a) * ones (idx));
    else
      if (strcmp (class (a), "double"))
	## This is faster with octave for double/Complex
	x = a * ones(idx, class(a));
      else
	cidx = cell (1, length (idx));
	for i=1:length(idx)
	  cidx{i} = ones (1,idx(i));
	endfor
	x = a (cidx{:});
      endif
    endif
  elseif (ndims (a) == 2 && length (idx) < 3)
    if (ischar (a))
      x = char (kron (ones (idx), toascii (a)));
    elseif (strcmp (class(a), "double"))
      ## FIXME -- DISPATCH.
      if (issparse (a))
        x = spkron (ones (idx), a);
      else
        x = kron (ones (idx), a);
      endif
    else
      aidx = size(a);
      x = a (kron (ones (1, idx(1)), 1:aidx(1)),  
	     kron (ones (1, idx(2)), 1:aidx(2)));
    endif
  else
    aidx = size(a);
    if (length(aidx) > length(idx))
      idx = [idx, ones(1,length(aidx)-length(idx))];
    elseif (length(aidx) < length(idx))
      aidx = [aidx, ones(1,length(idx)-length(aidx))];
    endif
    cidx = cell (1, length (aidx));
    for i=1:length(aidx)
      cidx{i} = kron (ones (1, idx(i)), 1:aidx(i));
    endfor
    x = a (cidx{:});
  endif

endfunction

# Test various methods of providing size parameters
%!shared x
%! x = [1 2;3 4];
%!assert(repmat(x, [1 1]), repmat(x, 1));
%!assert(repmat(x, [3 3]), repmat(x, 3));
%!assert(repmat(x, [1 1]), repmat(x, 1, 1));
%!assert(repmat(x, [1 3]), repmat(x, 1, 3));
%!assert(repmat(x, [3 1]), repmat(x, 3, 1));
%!assert(repmat(x, [3 3]), repmat(x, 3, 3));

# Tests for numel==1 case:
%!shared x, r
%! x = [ 65 ];
%! r = kron(ones(2,2), x);
%!assert(r, repmat(x, [2 2]));
%!assert(char(r), repmat(char(x), [2 2]));
%!assert(int8(r), repmat(int8(x), [2 2]));

# Tests for ndims==2 case:
%!shared x, r
%! x = [ 65 66 67 ];
%! r = kron(ones(2,2), x);
%!assert(r, repmat(x, [2 2]));
%!assert(char(r), repmat(char(x), [2 2]));
%!assert(int8(r), repmat(int8(x), [2 2]));

# Tests for dim>2 case:
%!shared x, r
%! x = [ 65 66 67 ];
%! r = kron(ones(2,2), x);
%! r(:,:,2) = r(:,:,1);
%!assert(r, repmat(x, [2 2 2]));
%!assert(char(r), repmat(char(x), [2 2 2]));
%!assert(int8(r), repmat(int8(x), [2 2 2]));

# Test that sparsity is kept
%!assert(sparse(4,4), repmat(sparse(2,2),[2 2]));

