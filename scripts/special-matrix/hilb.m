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

function retval = hilb (n)

# usage: hilb (n)
#
# Return the Hilbert matrix of order n.  The i, j element of a Hilbert
# matrix is defined as
#
#  H (i, j) = 1 / (i + j - 1);
#
# See also: hankel, vander, hadamard, invhilb, toeplitz


  if (nargin != 1)
    usage ("hilb (n)");
  endif

  nmax = length (n);
  if (nmax == 1)
    retval = zeros (n);
    tmp = 1:n;
    for i = 1:n
      retval (i, :) = 1.0 ./ (tmp + (i - 1));
    endfor
  else
    error ("hilb: expecting scalar argument, found something else");
  endif

endfunction
