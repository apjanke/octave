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
## Software Foundation, 59 Temple Place - Suite 330, Boston, MA
## 02111-1307, USA.

-*- texinfo -*-\n\
## @deftypefn {Built-in Function} {} puts (@var{string})
## Write a string to the standard output with no formatting.
## @end deftypefn

## See also: fputs, printf, fprintf

## Author: jwe

function retval = puts (s)

  retval = -1;

  if (nargin == 1)
    retval = fputs (stdout, s);
  else
    usage ("puts (string)");
  endif

endfunction
