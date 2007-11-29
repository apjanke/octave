## Copyright (C) 1993, 1994, 1995, 1996, 1997, 1999, 2000, 2003, 2004,
##               2005, 2006, 2007 John W. Eaton
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
## @deftypefn {Function File} {} grid (@var{arg})
## @deftypefnx {Function File} {} grid ("minor", @var{arg2})
## Force the display of a grid on the plot.
## The argument may be either @code{"on"} or @code{"off"}.  If it is
## omitted, the current grid state is toggled.
##
## If @var{arg} is @code{"minor"} then the minor grid is toggled.  When
## using a minor grid a second argument @var{arg2} is allowed, which can
## be either @code{"on"} or @code{"off"} to explicitly set the state of
## the minor grid.
## @seealso{plot}
## @end deftypefn

## Author: jwe

## PKG_ADD: mark_as_command grid

function grid (varargin)

  persistent grid_on = false;
  persistent minor_on = false;

  [ax, varargin, nargs] = __plt_get_axis_arg__ ("grid", varargin{:});
  if (nargs > 1)
    print_usage ();
  elseif (nargs == 0)
    grid_on = ! grid_on;
  else
    x = varargin{1};
    if (ischar (x))
      if (strcmp ("off", x))
	grid_on = false;
      elseif (strcmp ("on", x))
	grid_on = true;
      elseif (strcmp ("minor", x))
	minor_on = ! minor_on;
	if (minor_on)
	  grid_on = true;
	endif
      else
	print_usage ();
      endif
    else
      error ("grid: argument must be a string");
    endif
  endif

  if (grid_on)
    set (ax, "xgrid", "on", "ygrid", "on", "zgrid", "on");
    if (minor_on)
      set (ax, "xminorgrid", "on", "yminorgrid", "on", "zminorgrid", "on");
    else
      set (ax, "xminorgrid", "off", "yminorgrid", "off", "zminorgrid", "off");
    endif
  else
    set (ax, "xgrid", "off", "ygrid", "off", "zgrid", "off");
    set (ax, "xminorgrid", "off", "yminorgrid", "off", "zminorgrid", "off");
  endif

endfunction
