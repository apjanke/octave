## Copyright (C) 2006 John W. Eaton
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
## @deftypefn {Function File} {} box (@var{arg})
## @deftypefnx {Function File} {} grid (@var{h}, @dots{})
## Control the display of a border around the plot.
## The argument may be either @code{"on"} or @code{"off"}.  If it is
## omitted, the the current box state is toggled.
## @seealso{grid}
## @end deftypefn

## Author: jwe

## PKG_ADD: mark_as_command box

function box (varargin)

  h = gca ();

  box_state = get (h, "box");

  nargs = numel (varargin);

  if (nargs == 0)
    if (strcmp (box_state, "on"))
      box_state = "off";
    else
      box_state = "on";
    endif
  elseif (nargs == 1)
    state = varargin{1};
    if (ischar (state))
      if (strcmp ("off", state))
	box_state = "off";
      elseif (strcmp ("on", state))
	box_state = "on";
      else
	print_usage ();
      endif
    endif
  else
    print_usage ();
  endif

  set (h, "box", box_state);

endfunction
