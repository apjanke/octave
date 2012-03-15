## Copyright (C) 2005-2012 S�ren Hauberg
## Copyright (C) 2010 VZLU Prague, a.s.
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
## @deftypefn  {Function File} {@var{text} =} split_by (@var{text}, @var{sep})
## Undocumented internal function.
## @end deftypefn

## Split the text into a cell array of strings by sep.
## Example: "A, B" => {"A", "B"} (with sep = ",")
function out = split_by (text, sep)
  out = strtrim (strsplit (text, sep));
endfunction

