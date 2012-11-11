## Copyright (C) 1994-2012 John W. Eaton
## Copyright (C) 2012 Carnë Draug
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
## @deftypefn  {Function File} {[@var{x}, @var{map}] =} rgb2ind (@var{rgb})
## @deftypefnx {Function File} {[@var{x}, @var{map}] =} rgb2ind (@var{R}, @var{G}, @var{B})
## Convert an image in red-green-blue (RGB) space to an indexed image.
## @seealso{ind2rgb, rgb2hsv, rgb2ntsc}
## @end deftypefn

## Bugs: The color map may have duplicate entries.

## Author: Tony Richardson <arichard@stark.cc.oh.us>
## Created: July 1994
## Adapted-By: jwe

function [x, map] = rgb2ind (R, G, B)

  if (nargin != 1 && nargin != 3)
    print_usage ();
  endif

  if (nargin == 1)
    rgb = R;
    if (length (size (rgb)) == 3 && size (rgb, 3) == 3)
      R = rgb(:,:,1);
      G = rgb(:,:,2);
      B = rgb(:,:,3);
    else
      error ("rgb2ind: argument is not an RGB image");
    endif
  endif

  if (! size_equal (R, G) || ! size_equal (R, B))
    error ("rgb2ind: arguments must all have the same size");
  endif

  map = unique([R(:) G(:) B(:)], "rows");
  ## RGB values for each point of the image are listed and expanded to compare
  ## with the colormap values. We then find the indexes when all 3 match and
  ## rebuild the image to its original size
  [x, ~] = find (squeeze (all (bsxfun (@eq, reshape(map', [1 3 rows(map)]), [R(:) G(:) B(:)]), 2))');
  x = reshape (x, size (R));

  ## a colormap is of class double and values between 0 and 1
  switch class (R)
    case {"single", "double", "logical"}
      ## do nothing, return the same
    case {"uint8", "uint16"}
      map = double (map) / double (intmax (class (R)));
    case "int16"
      map = (double (im) + 32768) / 65535;
    otherwise
      error ("unsupported image class %s", im_class);
  endswitch

  ## we convert to the smallest class necessary to encode the image. Matlab
  ## documentation does not mention what it does when uint16 is not enough...
  ## When an indexed image is of integer class, there's a -1 offset to the
  ## colormap, hence the adjustment
  if (rows (map) < 256)
    x = uint8 (x - 1);
  elseif (rows (map) < 65536)
    x = uint16 (x - 1);
  else
    ## leave it as double
  endif
endfunction
