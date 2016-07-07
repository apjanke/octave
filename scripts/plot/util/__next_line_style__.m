## Copyright (C) 2010-2015 David Bateman
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
## @deftypefn {} {@var{style} =} __next_line_style__ (@var{reset})
## Undocumented internal function.
## @end deftypefn

## Return the next line style in the rotation.


function [linestyle, marker] = __next_line_style__ ()

  ca = gca ();

  styleorder = get (ca, "linestyleorder");
  if (isempty (styleorder))
    linestyle = "-";   # basic line
    marker = "none";   # no marker
    return;
  endif

  if (ischar (styleorder))
    styleorder = cellstr (styleorder);
  endif

  style_idx = fix (get (ca, "linestyleorderindex"));
  num_styles = rows (styleorder);
  if (style_idx > num_styles)
    style_idx = mod (style_idx, num_styles);
  endif
  if (style_idx < 1)
    style_idx = 1;
  endif

  options = __pltopt__ ("__next_line_style__", styleorder{style_idx});
  linestyle = options.linestyle;
  marker = options.marker;

endfunction


%!test
%! hf = figure ("visible", "off");
%! unwind_protect
%!   hax = axes ();
%!   set (hax, "colororder", [0 0 1]);
%!   set (hax, "linestyleorder", {"-", ":", "--"});
%!   hold on;
%!   h = plot (1:5,1:5, 1:4,1:4, 1:3,1:3);
%!   assert (get (h, "linestyle"), {"-"; ":"; "--"});
%!   cla (hax);
%!   hold on;
%!   h1 = plot (1:5,1:5);
%!   h2 = plot (1:4,1:4);
%!   h3 = plot (1:3,1:3);
%!   assert (get ([h1;h2;h3], "linestyle"), {"-"; ":"; "--"});
%! unwind_protect_cleanup
%!   close (hf);
%! end_unwind_protect

