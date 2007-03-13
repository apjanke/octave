## Copyright (C) 2007 John W. Eaton
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
## @deftypefn {Function File} {} cast (@var{val}, @var{type})
## Convert @var{val} to data type @var{type}.
## @seealso{int8, uint8, int16, uint16, int32, uint32, int64, uint64, double}
## @end deftypefn

## Author: jwe

function retval = cast (val, typ)

  if (nargin == 2)
    if (ischar (typ))
      if (any (strcmp (typ, {"int8"; "uint8"; "int16"; "uint16";
			     "int32"; "uint32"; "int64"; "uint64";
			     "double"; "single"})))
	retval = feval (typ, val);
      else
	error ("cast: type name `%s' is not a built-in type", typ);
      endif
    else
      error ("cast: expecting type name as second argument");
    endif
  else
    print_usage ();
  endif

endfunction
