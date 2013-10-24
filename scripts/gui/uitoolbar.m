## Copyright (C) 2012-2013 Michael Goffioul
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
## @deftypefn  {Function File} {@var{handle} =} uitoolbar ("Name", value, @dots{})
## @deftypefnx {Function File} {@var{handle} =} uitoolbar (@var{parent}, "Name", value, @dots{})
## @end deftypefn

## Author: goffioul

function handle = uitoolbar (varargin)

  [h, args] = __uiobject_split_args__ ("uitoolbar", varargin, {"figure"});
  handle = __go_uitoolbar__ (h, args{:});

endfunction

