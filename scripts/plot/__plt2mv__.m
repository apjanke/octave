## Copyright (C) 1996, 1997 John W. Eaton
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

## Undocumented internal function.

## Author: jwe

function retval = __plt2mv__ (h, x, y, options, properties)

  if (nargin < 3 || nargin > 5)
    print_usage ();
  endif

  if (nargin < 4 || isempty (options))
    options = __default_plot_options__ ();
  endif

  if (nargin < 5)
    properties = {};
  endif

  [x_nr, x_nc] = size (x);
  [y_nr, y_nc] = size (y);

  if (y_nr == 1)
    y = y';
    tmp = y_nr;
    y_nr = y_nc;
    y_nc = tmp;
  endif

  if (x_nr == y_nr)
    1;
  elseif (x_nc == y_nr)
    x = x';
    tmp = x_nr;
    x_nr = x_nc;
    x_nc = tmp;
  else
    error ("__plt2mv__: matrix dimensions must match");
  endif

  if (x_nc > 0)
    if (numel (options) == 1)
      options = repmat (options(:), x_nc, 1);
    endif
    retval = zeros (x_nc, 1);
    for i = 1:x_nc
      tkey = options(i).key;
      if (! isempty (tkey))
	set (h, "key", "on");
      endif
      color = options(i).color;
      if (isempty (color))
	color = __next_line_color__ ();
      endif
      retval(i) = line (x(:,i), y, "keylabel", tkey, "color", color,
			"linestyle", options(i).linestyle,
			"marker", options(i).marker, properties{:});
    endfor
  else
    error ("__plt2mv__: arguments must be a matrices");
  endif

endfunction
