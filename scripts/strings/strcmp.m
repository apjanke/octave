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

## usage: strcmp (s1, s2)
##
## Compare two strings.  Trailing blanks are significant.
##
## WARNING:  Unlike the C function of the same name, this function
## returns 1 for equal and zero for not equal.  Why?  To be compatible
## with Matlab, of course.
##
## Why doesn't this always return a scalar instead of vector with
## elements corresponding to the rows of the string array?  To be
## compatible with Matlab, of course.

## Author: jwe

function status = strcmp (s1, s2)

  if (nargin != 2)
    usage ("strcmp (s, t)");
  endif

  status = 0;
  if (isstr (s1) && isstr(s2))
    c1 = columns (s1);
    c2 = columns (s2);
    if (c1 == c2)
      if (c1 == 0)
        status = 1;
      else
        status = all (all (s1 == s2));
      endif
    endif
  endif

endfunction
