# Copyright (C) 1997 Kai P. Mueller
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

function retval = is_abcd(a, b, c, d)
  # ------------------------------------------------------
  # retval = is_abcd(a [, b, c, d])
  # Returns retval = 1 if the dimensions of a, b, c, d
  # are compatible, otherwise retval = 0.
  # The matrices b, c, or d may be omitted.
  # ------------------------------------------------------
  # 
  # see also: abcddim

  # Written by Kai P. Mueller November 4, 1997
  # based on is_controllable.m of Scottedward Hodel
  # modified by
  # $Revision: 2.0.0.0 $

  retval = 0;
  switch (nargin)
    case (1)
      # A only
      [na, ma] = size(a);
      if (na != ma)
        disp("Matrix A ist not square.")
      endif
    case (2)
      # A, B only
      [na, ma] = size(a);  [nb, mb] = size(b);
      if (na != ma)
        disp("Matrix A ist not square.")
	return;
      endif
      if (na != nb)
        disp("A and B column dimension different.")
        return;
      endif
    case (3)
      # A, B, C only
      [na, ma] = size(a);  [nb, mb] = size(b);  [nc, mc] = size(c);
      if (na != ma)
        disp("Matrix A ist not square.")
	return;
      endif
      if (na != nb)
        disp("A and B column dimensions not compatible.")
	return;
      endif
      if (ma != mc)
        disp("A and C row dimensions not compatible.")
	return;
      endif
    case (4)
      # all matrices A, B, C, D
      [na, ma] = size(a);  [nb, mb] = size(b);
      [nc, mc] = size(c);  [nd, md] = size(d);
      if (na != ma)
        disp("Matrix A ist not square.")
	return;
      endif
      if (na != nb)
        disp("A and B column dimensions not compatible.")
	return;
      endif
      if (ma != mc)
        disp("A and C row dimensions not compatible.")
	return;
      endif
      if (mb != md)
        disp("B and D row dimensions not compatible.")
	return;
      endif
      if (nc != nd)
        disp("C and D column dimensions not compatible.")
	return;
      endif
    otherwise
      usage("retval = is_abcd(a [, b, c, d])")
  endswitch
  # all tests passed, signal ok.
  retval = 1;
endfunction
