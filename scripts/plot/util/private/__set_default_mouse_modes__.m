## Copyright (C) 2017 Rik Wehbring
##
## This file is part of Octave.
##
## Octave is free software: you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <https://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn {} {} __set_default_mouse_modes__.m (@var{fig})
## Set mouse mode properties of figure to default values.
##
## @end deftypefn

function __set_default_mouse_modes__ (fig)

  set (fig, "__pan_mode__", struct ("Enable", "off",
                                    "Motion", "both",
                                    "FigureHandle", fig),
            "__rotate_mode__", struct ("Enable", "off",
                                       "RotateStyle", "box",
                                       "FigureHandle", fig),
            "__zoom_mode__", struct ("Enable", "off",
                                     "Motion", "both",
                                     "Direction", "in",
                                     "FigureHandle", fig));

endfunction
