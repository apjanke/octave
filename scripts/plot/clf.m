## Copyright (C) 2005, 2006, 2007 John W. Eaton
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
## @deftypefn {Function File} {} clf ()
## Clear the current figure.
## @seealso{close, delete}
## @end deftypefn

## Author: jwe

function clf ()

  if (nargin == 0)
    cf = gcf ();
    set (cf, "currentaxes", []);
    for k = get (cf, "children")
      if (ishandle (k))
        delete (k);
      endif
    endfor
  else
    print_usage ();
  endif

endfunction
