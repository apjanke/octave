# Copyright (C) 1993, 1994 John W. Eaton
# 
# This file is part of Octave.
# 
# Octave is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any
# later version.
# 
# Octave is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Octave; see the file COPYING.  If not, write to the Free
# Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

function retval = is_matrix (x)

# usage: is_matrix (x)
#
# Return 1 if the number of rows and columns of x are both greater
# than 1.
#
# See also: size, rows, columns, length, is_scalar, is_vector

  if (nargin == 1)
    [nr, nc] = size (x);
    retval = (nr > 1 && nc > 1);
  else
    usage ("is_matrix (x)");
  endif

endfunction
