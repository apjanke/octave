## Copyright (C) 2009 Kai Habel
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
## @deftypefn {Function File} {} specular (@var{sx}, @var{sy}, @var{sz}, @var{l}, @var{v})
## @deftypefnx {Function File} {} specular (@var{sx}, @var{sy}, @var{sz}, @var{l}, @var{v}, @var{se})
## Calculate specular reflection strength of a surface defined by the normal
## vector elements @var{sx}, @var{sy}, @var{sz} using Phong's approximation. 
## The light and view vectors can be specified using parameter @var{L} and @var{V} respectively.
## Both can be given as 2-element vectors [azimuth, elevation] in degrees or as 3-element
## vector [x, y, z]. An optional 6th argument describes the specular exponent (spread) @var{se}.
## @seealso{surfl, diffuse}
## @end deftypefn

## Author: Kai Habel <kai.habel@gmx.de>

function retval = specular (sx, sy, sz, lv, vv, se)

  ## general checks
  if ((nargin < 5) || (nargin > 6))
    usage ("number of arguments must be 5 or 6")
  endif

  ## checks for specular exponent (se)
  if (nargin < 6)
    se = 10;
  else
    if (!isnumeric (se) || (numel (se) != 1) || (se <= 0))
      usage ("specular exponent must be positive scalar");
    endif
  endif

  ## checks for normal vector
  if (!size_equal (sx, sy, sz))
    usage ("SX, SY, and SZ must have same size")
  endif
  
  ## check for light vector (lv) argument
  if (length (lv) < 2 || length (lv) > 3)
    usage ("light vector LV must be a 2- or 3-element vector");
  elseif (length (lv) == 2)
    [lv(1), lv(2), lv(3)] = sph2cart (lv(1) * pi/180, lv(2) * pi/180, 1.0);
  endif

  ## check for view vector (vv) argument
  if ((length (vv) < 2) || (length (lv) > 3))
    error ("view vector VV must be a 2- or 3-element vector");
  elseif (length (vv) == 2)
    [vv(1), vv(2), vv(3)] = sph2cart (vv(1) * pi / 180, vv(2) * pi / 180, 1.0);
  endif

  ## normalize view and light vector
  if (sum (abs (lv)) > 0)
    lv  /= norm (lv);
  endif
  if (sum (abs (vv)) > 0)
    vv  /= norm (vv);
  endif

  ## calculate normal vector lengths and dot-products
  ns = sqrt (sx.^2 + sy.^2 + sz.^2);
  l_dot_n = (sx * lv(1) + sy * lv(2) + sz * lv(3)) ./ ns;
  v_dot_n = (sx * vv(1) + sy * vv(2) + sz * vv(3)) ./ ns;

  ## calculate specular reflection using Phong's approximation
  retval = 2 * l_dot_n .* v_dot_n - dot (lv, vv);
  
  ## set zero if light is on the other side
  retval(l_dot_n < 0) = 0;

  ## allow postive values only
  retval(retval < 0) = 0;
  retval = retval .^ se;
  
endfunction
