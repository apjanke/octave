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

function plot_2_m_m (x, y)

  if (nargin != 2)
    usage ("plot_2_m_m (x, y)");
  endif

  [x_nr, x_nc] = size (x);
  [y_nr, y_nc] = size (y);

  if (x_nr == y_nr && x_nc == y_nc)
    if (x_nc > 0)
      tmp = [x, y];
      command = sprintf ("gplot tmp(:,%d:%d:%d)", 1, x_nc, x_nc+1);
      for i = 2:x_nc
        command = sprintf ("%s, tmp(:,%d:%d:%d)", command, i, x_nc, x_nc+i);
      endfor
      eval (command);
    else
      error ("plot_2_m_m: arguments must be a matrices");
    endif
  else
    error ("plot_2_m_m: matrix dimensions must match");
  endif

endfunction
