## Copyright (C) 2003 Alois Schloegl
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
##
## Copyright (C) 2000 Paul Kienzle

## usage: strmatch(s, A [, 'exact'])
## Determines which entries of A match string s. A can be a string matrix
## or a cell array of strings. If 'exact' is not given, then s only needs 
## to match A up to the length of s. Null characters match blanks.
## Results are returned as a column vector.
function idx = strmatch(s,A,exact)
  if (nargin < 2 || nargin > 3)
    usage("strmatch(s,A,'exact')");
  endif

  try istno = implicit_str_to_num_ok;
  catch istno = 0;
  end
  try wstno = warn_str_to_num;
  catch wstno = 0;
  end
  try dfi = do_fortran_indexing;
  catch dfi = 0;
  end
  try wfi = warn_fortran_indexing;
  catch wfi = 0;
  end
  unwind_protect
    implicit_str_to_num_ok = 1;
    warn_str_to_num = 0;
    do_fortran_indexing = 1;
    warn_fortran_indexing = 0;

    [nr, nc] = size (A);
    if iscell(A)
      match = zeros(prod(size(A)),1);
      if nargin>2,
        for k = 1:prod(size(A)),
          match(k) = strcmp(s,A{k}); 
        end 
      else
        for k = 1:prod(size(A)),
          match(k) = strncmp(s,A{k},length(s));
        end
      end
      idx = find(match);
    elseif (length (s) > nc)
      idx = [];
    else
      if (nargin == 3 && length(s) < nc) s(1,nc) = ' '; endif
      s (s==0) = ' ';
      A (A==0) = ' ';
      match = s(ones(size(A,1),1),:) == A(:,1:length(s));
      if (length(s) == 1)
	idx = find(match);
      else
      	idx = find(all(match')');
      endif
    endif
    
  unwind_protect_cleanup
    implicit_str_to_num_ok = istno;
    warn_str_to_num = wstno;
    do_fortran_indexing = dfi;
    warn_fortran_indexing = wfi;
  end_unwind_protect
endfunction 
