# Copyright (C) 1996 A. Scottedward Hodel 
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
 
function B = swaprows(A)
  # function B = swaprows(A)
  # permute rows of A into reverse order

  # A. S. Hodel July 23, 1992
  # Conversion to Octave R. Bruce Tenison July 4, 1994
  # $Revision: 2.0.0.0 $
  
  m = rows(A);
  idx = m:-1:1;
  B = A(idx,:);
endfunction

