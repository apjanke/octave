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

function axis (ax)

# usage: axis ()
#        axis ([xmin, xmax])
#        axis ([xmin, xmax, ymin, ymax])
#        axis ([xmin, xmax, ymin, ymax, zmin, zmax])
#
# Sets the axis limits.
#
# With no arguments, turns autoscaling on.
#
# If your plot is already drawn, then you need to REPLOT before 
# the new axis limits will take effect.

  if (nargin > 1)
    usage ("axis ([xmin, xmax, ymin, ymax, zmin, zmax])");
  endif

  if (nargin == 0)
    set autoscale;
  elseif (is_vector (ax))

    len = length (ax);

    if (len != 2 && len != 4 && len != 6)
      error ("axis: expecting vector with 2, 4, or 6 elements");
    endif

    if (len > 1)
      eval (sprintf ("set xrange [%g:%g];", ax (1), ax (2)));
    endif

    if (len > 3)
      eval (sprintf ("set yrange [%g:%g];", ax (3), ax (4)));
    endif

    if (len > 5)
      eval (sprintf ("set zrange [%g:%g];", ax (5), ax (6)));
    endif

  else
    error ("axis: expecting no args, or a vector with 2, 4, or 6 elements");
  endif

endfunction
