## Copyright (C) 1994-2012 John W. Eaton
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
## @deftypefn  {Function File} {@var{map} =} ocean ()
## @deftypefnx {Function File} {@var{map} =} ocean (@var{n})
## Create color colormap.  This colormap varies from black to white with shades
## of blue.
## The argument @var{n} must be a scalar.
## If unspecified, the length of the current colormap, or 64, is used.
## @end deftypefn

## Author: Tony Richardson <arichard@stark.cc.oh.us>
## Created: July 1994
## Adapted-By: jwe

function map = ocean (n)

  if (nargin == 0)
    n = rows (colormap);
  elseif (nargin == 1)
    if (! isscalar (n))
      error ("ocean: argument must be a scalar");
    endif
  else
    print_usage ();
  endif

  cutin = fix (n/3);

  dr = (n - 1) / cutin;

  r = prepad ([0:dr:(n-1)], n)';

  dg = (n - 1) / (2 * cutin);

  g = prepad([0:dg:(n-1)], n)';

  b = [0:(n-1)]';

  map = [ r, g, b ] / (n - 1);

endfunction


%!demo
%! ## Show the 'ocean' colormap as an image
%! image (1:64, linspace (0, 1, 64), repmat ((1:64)', 1, 64));
%! axis ([1, 64, 0, 1], "ticy", "xy");
%! colormap (ocean (64));

