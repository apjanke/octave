## Copyright (C) 2005 John W. Eaton
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
## @deftypefn {Function File} {} newplot ()
## @end deftypefn

function newplot ()

  if (nargin == 0)
    cf = gcf ();
    fnp = get (cf, "nextplot");
    switch (fnp)
      case "new"
      case "add"
	return;
      case "replacechildren"
      case "replace"
      otherwise
	error ("newplot: unrecognized nextplot property for current figure");
    endswitch
    ca = gca ();
    anp = get (ca, "nextplot");
    switch (anp)
      case "add"
      case "replacechildren"
      case "replace"
	__next_line_color__ (true);
	__uiobject_axes_init__ (ca);
      otherwise
	error ("newplot: unrecognized nextplot property for current axes");
    endswitch
  else
    print_usage ();
  endif

endfunction
