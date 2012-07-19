## Copyright (C) 2012 pdiribarne
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <http://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn   {Function File} {@var{hnew} =} copyobj (@var{horig})
## @deftypefnx  {Function File} {@var{hnew} =} copyobj (@var{horig}, @var{hparent})
## Constructs a copy of the object associated with handle @var{horig}
## and returns a handle, @var{hnew}, to the new object.
## If a parent handle @var{hparent} (root, figure, axes or hggroup) is specified,
## the copied object will be created as a child to @var{hparent}.
## @seealso{findobj, get, set, struct2hdl, hdl2struct}
## @end deftypefn

## Author: pdiribarne <pdiribarne@new-host.home>
## Created: 2012-04-01

function hout = copyobj (hin, hpar = 0)

  partypes = {"root", "figure", "axes", "hggroup"};
  othertypes = {"line", "patch", "surface", "image", "text"};
  alltypes = [partypes othertypes];

  if (! ishandle (hin) || nargin > 2)
    print_usage ();
  elseif (! ishandle (hpar))
    hpar = figure (floor (hpar));
  elseif (! any (strcmpi (get (hpar).type, partypes)))
    print_usage ();
  endif

  ## compatibility of input handles
  kididx = find (strcmp (alltypes, get (hin).type));
  paridx = find (strcmp (alltypes, get (hpar).type));

  if (kididx <= paridx)
    error ("copyobj: %s object can't be children to %s.",
           alltypes{kididx}, alltypes{paridx})
  elseif nargin == 1
    str = hdl2struct (hin);
    hout = struct2hdl (str);
  else
    str = hdl2struct (hin);
    hout = struct2hdl (str, hpar);
  endif
endfunction

%!test
%! h1 = figure ();
%! set (h1, "visible", "off")
%! x = 0:0.1:2*pi;
%! y1 = sin (x);
%! y2 = exp (x - 1);
%! ax = plotyy (x,y1, x-1,y2, @plot, @semilogy);
%! xlabel ("X");
%! ylabel (ax(1), "Axis 1");
%! ylabel (ax(2), "Axis 2");
%! axes (ax(1));
%! text (0.5, 0.5, "Left Axis", ...
%!       "color", [0 0 1], "horizontalalignment", "center");
%! axes (ax(2));
%! text (4.5, 80, "Right Axis", ...
%!       "color", [0 0.5 0], "horizontalalignment", "center");
%! s1 = hdl2struct (h1);
%! h2 = struct2hdl (s1);
%! s2 = hdl2struct (h2);
%! png1 = strcat (tmpnam (), ".png");
%! png2 = strcat (tmpnam (), ".png");
%! unwind_protect
%!   print (h1, png1)
%!   [img1, map1, alpha1] = imread (png1);
%!   print (h2, png2)
%!   [img2, map2, alpha2] = imread (png2);
%! unwind_protect_cleanup
%!   unlink (png1);
%!   unlink (png2);
%! end_unwind_protect
%! assert (img1, img2)
%! assert (map1, map2)
%! assert (alpha1, alpha2)

%!demo
%! hdl = figure (1234);
%! clf ()
%! hold on
%! x = 1:10;
%! y = x.^2;
%! dy = 2 * (.2 * x);
%! y2 = (x - 3).^2;
%! hg = errorbar (x, y, dy,'#~');
%! set (hg, 'marker', '^', 'markerfacecolor', rand(1,3))
%! plot (x, y2, 'ok-')
%! legend ('errorbar', 'line')
%! hout = copyobj (1234);

%!demo
%! hdl = figure (1234);
%! clf ()
%! subplot (2, 2, 1);
%! hold on
%! [C, H] = contourf (rand(10, 10));
%! colorbar
%! subplot (2, 2, 2);
%! hold on
%! quiver (rand(10, 10), rand(10, 10))
%! subplot (2, 2, 3);
%! colormap (jet (64))
%! sombrero;
%! colorbar('peer', gca, 'NorthOutside')
%! subplot (2, 2, 4);
%! imagesc (rand (30, 30));
%! text (15, 15, 'Rotated text', ...
%!      'HorizontAlalignment', 'Center', 'Rotation', 30);
%! hout = copyobj (1234);

