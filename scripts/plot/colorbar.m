## Copyright (C) 2008 David Bateman
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
## @deftypefn {Function File} {} colorbar (@var{s})
## @deftypefnx {Function File} {} colorbar ('peer', @var{h}, @dots{})
## Adds a colorbar to the current axes. Valid values for @var{s} are
##
## @table @asis
## @item 'EastOutside'
## Place the colorbar outside the plot to the right. This is the default.
## @item 'East'
## Place the colorbar inside the plot to the right.
## @item 'WestOutside'
## Place the colorbar outside the plot to the left.
## @item 'West'
## Place the colorbar inside the plot to the left.
## @item 'NorthOutside'
## Place the colorbar above the plot.
## @item 'North'
## Place the colorbar at the top of the plot.
## @item 'SouthOutside'
## Place the colorbar under the plot.
## @item 'South'
## Place the colorbar at the bottom of the plot.
## @item 'Off', 'None'
## Remove any existing colorbar from the plot.
## @end table
##
## If the argument 'peer' is given, then the following argument is treated
## as the axes handle on which to add the colorbar.
## @end deftypefn


## PKG_ADD: mark_as_command colorbar

function h = colorbar (varargin)
  ax = [];
  loc = "eastoutside";
  args = {};
  deleting = false;

  i = 1;
  while (i <= nargin)
    arg = varargin {i++};

    if (ischar(arg))
      if (strcmpi (arg, "peer"))
	if (i > nargin)
	  error ("colorbar: missing axes handle after 'peer'");
	else
	  ax = vargin{i++}
	  if (!isscalar (ax) || ! ishandle (ax)
	      || strcmp (get (ax, "type"), "axes"))
	    error ("colorbar: expecting an axes handle following 'peer'");
	  endif
	endif
      elseif (strcmpi (arg, "north") || strcmpi (arg, "south")
	      || strcmpi (arg, "east") || strcmpi (arg, "west")
	      || strcmpi (arg, "northoutside") || strcmpi (arg, "southoutside")
	      || strcmpi (arg, "eastoutside") || strcmpi (arg, "westoutside"))
	loc = arg;
      elseif (strcmpi (arg, "off") || strcmpi (arg, "none"))
	deleting = true;
      else
	args{end+1} = arg;
      endif
    else
      args{end+1} = arg;
    endif
  endwhile

  if (isempty (ax))
    ax = gca ();
  endif
  obj = get (ax);

  if (deleting)
    objs = findobj (get (ax, "parent"), "type", "axes");
    for i = 1 : length (objs)
      if (strcmp (get (objs(i), "tag"), "colorbar") &&
	  get (objs(i), "axes") == ax)
	delete (objs(i));
      endif
    endfor
    else
    position = obj.position;
    clen = rows (get (get (ax, "parent"), "colormap"));
    cext = get (ax, "clim");
    cdiff = (cext(2) - cext(1)) / clen / 2;
    cmin = cext(1) + cdiff;
    cmax = cext(2) - cdiff;

    orig_pos = obj.position;
    orig_opos = obj.outerposition;
    [pos, cpos, vertical, mirror, aspect] =  ...
	__position_colorbox__ (loc, obj, ancestor (ax, "figure"));
    set (ax, "activepositionproperty", "position", "position", pos);

    cax = __go_axes__ (get (ax, "parent"), "tag", "colorbar", 
		       "handlevisibility", "off", 
		       "activepositionproperty", "position", 
		       "position", cpos);
    addproperty ("location", cax, "radio",
		 "eastoutside|east|westoutside|west|northoutside|north|southoutside|south",
		 loc);
    addproperty ("axes", cax, "handle", ax);

    if (vertical)
      hi = image (cax, [0,1], [cmin, cmax], [1 : clen]');
      if (mirror)
	set (cax, "xtick", [], "xdir", "normal", "ydir", "normal",
	     "ylim", cext, "ylimmode", "manual",
	     "yaxislocation", "right", args{:});
      else
	set (cax, "xtick", [], "xdir", "normal", "ydir", "normal",
	     "ylim", cext, "ylimmode", "manual",
	     "yaxislocation", "left", args{:});
      endif
    else
      hi = image (cax, [cmin, cmax], [0,1], [1 : clen]);
      if (mirror)
	set (cax, "ytick", [], "xdir", "normal", "ydir", "normal",
	     "xlim", cext, "xlimmode", "manual",
	     "xaxislocation", "top", args{:});
      else
	set (cax, "ytick", [], "xdir", "normal", "ydir", "normal",
	     "xlim", cext, "xlimmode", "manual",
	     "xaxislocation", "bottom", args{:});
      endif
    endif

    if (! isnan (aspect))
      set (cax, "dataaspectratio", aspect);
    endif

    ctext = text (0, 0, "", "tag", "colorbar","visible", "off", 
		  "handlevisibility", "off", "xliminclude", "off",  
		  "yliminclude", "off", "zliminclude", "off",
		  "deletefcn", {@deletecolorbar, cax, orig_pos, orig_opos});

    set (cax, "deletefcn", {@resetaxis, orig_pos, orig_opos});

    addlistener (ax, "clim", {@update_colorbar_clim, hi, vertical})
    addlistener (ax, "dataaspectratio", {@update_colorbar_axis, cax})
    addlistener (ax, "position", {@update_colorbar_axis, cax})

  endif

  if (nargout > 0)
    h = cax;
  endif
endfunction

function deletecolorbar (h, d, hc, pos, opos)
  ## Don't delete the colorbar and reset the axis size if the
  ## parent figure is being deleted.
  if (ishandle (hc) && strcmp (get (hc, "type"), "axes") && 
      (isempty (gcbf()) || strcmp (get (gcbf(), "beingdeleted"),"off")))
    if (strcmp (get (hc, "beingdeleted"), "off"))
      delete (hc);
    endif
    if (!isempty (ancestor (h, "axes")) &&
	strcmp (get (ancestor (h, "axes"), "beingdeleted"), "off"))
      set (ancestor (h, "axes"), "position", pos, "outerposition", opos);
    endif
  endif
endfunction

function resetaxis (h, d, pos, opos)
  if (ishandle (h) && strcmp (get (h, "type"), "axes") && 
      (isempty (gcbf()) || strcmp (get (gcbf(), "beingdeleted"),"off")) &&
      ishandle (get (h, "axes")))
     set (get (h, "axes"), "position", pos, "outerposition", opos);
  endif
endfunction

function update_colorbar_clim (h, d, hi, vert)
  if (ishandle (h) && strcmp (get (h, "type"), "image") && 
      (isempty (gcbf()) || strcmp (get (gcbf(), "beingdeleted"),"off")))
    clen = rows (get (get (h, "parent"), "colormap"));
    cext = get (h, "clim");
    cdiff = (cext(2) - cext(1)) / clen / 2;
    cmin = cext(1) + cdiff;
    cmax = cext(2) - cdiff;

    if (vert)
      set (hi, "ydata", [cmin, cmax]);
      set (get (hi, "parent"), "ylim", cext);
    else
      set (hi, "xdata", [cmin, cmax]);
      set (get (hi, "parent"), "xlim", cext);
    endif
  endif
endfunction

function update_colorbar_axis (h, d, cax)
  if (ishandle (cax) && strcmp (get (cax, "type"), "axes") && 
      (isempty (gcbf()) || strcmp (get (gcbf(), "beingdeleted"),"off")))
    loc = get (cax, "location");
    obj = get (h);
    [pos, cpos, vertical, mirror, aspect] =  ...
	__position_colorbox__ (loc, obj, ancestor (h, "figure"));

    if (vertical)
      if (mirror)
	set (cax, "xtick", [], "xdir", "normal", "ydir", "normal",
	     "yaxislocation", "right", "position", cpos);
      else
	set (cax, "xtick", [], "xdir", "normal", "ydir", "normal",
	     "yaxislocation", "left", "position", cpos);
      endif
    else
      if (mirror)
	set (cax, "ytick", [], "xdir", "normal", "ydir", "normal",
	     "xaxislocation", "top", "position", cpos);
      else
	set (cax, "ytick", [], "xdir", "normal", "ydir", "normal",
	     "xaxislocation", "bottom", "position", cpos);
      endif
    endif

    if (! isnan (aspect))
      aspect
      set (cax, "dataaspectratio", aspect);
    endif
  endif
endfunction

function [pos, cpos, vertical, mirr, aspect] = __position_colorbox__ (cbox, obj, cf)

  pos = obj.position;
  sz = pos(3:4);

  off = 0;
  if (strcmpi (obj.dataaspectratiomode, "manual"))
    r = obj.dataaspectratio;
    if (pos(3) > pos(4))
      switch (cbox)
	case {"east", "eastoutside", "west", "westoutside"}
	  off = [(pos(3) - pos(4)) ./ (r(2) / r(1)), 0];	  
      endswitch
    else
      switch (cbox)
	case {"north", "northoutside", "south", "southoutside"}
	  off = [0, (pos(4) - pos(3)) ./ (r(1) / r(2))];
	  ## This shouldn't be here except that gnuplot doesn't have a
	  ## square window and so a square aspect ratio is not square.
	  ## The corrections are empirical.
	  if (strcmp (get (cf, "__backend__"), "gnuplot"))
	    if (length (cbox) > 7 && strcmp (cbox(end-6:end),"outside"))
	      off = off / 2;
	    else
	      off = off / 1.7;
	    endif
	  endif
      endswitch
    endif
    off = off / 2;
  endif

  switch (cbox)
    case "northoutside"
      origin = pos(1:2) + [0., 0.9] .* sz + [1, -1] .* off;
      sz = sz .* [1.0, 0.06];
      pos(4) = 0.8 * pos(4);
      mirr = true;
      vertical = false;
    case "north"
      origin = pos(1:2) + [0.05, 0.9] .* sz + [1, -1] .* off;
      sz = sz .* [1.0, 0.06] * 0.9;
      mirr = false;
      vertical = false;
    case "southoutside"
      origin = pos(1:2) + off;
      sz = sz .* [1.0, 0.06];
      pos(2) = pos(2) + pos(4) * 0.2;
      pos(4) = 0.8 * pos(4);
      mirr = false;
      vertical = false;
    case "south"
      origin = pos(1:2) + [0.05, 0.05] .* sz + off;
      sz = sz .* [1.0, 0.06] * 0.9;
      mirr = true;
      vertical = false;
    case "eastoutside"
      origin = pos(1:2) + [0.9, 0] .* sz + [-1, 1] .* off;
      sz = sz .* [0.06, 1.0];
      pos(3) = 0.8 * pos(3);
      mirr = true;
      vertical = true;
    case "east"
      origin = pos(1:2) + [0.9, 0.05] .* sz + [-1, 1] .* off;
      sz = sz .* [0.06, 1.0] * 0.9;
      mirr = false;
      vertical = true;
    case "westoutside"
      origin = pos(1:2) + off;
      sz = sz .* [0.06, 1.0];
      pos(1) = pos(1) + pos(3) * 0.2;
      pos(3) = 0.8 * pos(3);
      mirr = false;
      vertical = true;
    case "west"
      origin = pos(1:2) + [0.05, 0.05] .* sz + off;
      sz = sz .* [0.06, 1.0] .* 0.9;
      mirr = true;
      vertical = true;
  endswitch

  cpos = [origin, sz];

  if (strcmpi (obj.dataaspectratiomode, "manual"))
    r = obj.dataaspectratio;

    if (pos(3) > pos(4))
      if (vertical)
	aspect = [1, 0.21, 1];
      else
	aspect = [0.21, 1, 1];
      endif
    else
      if (vertical)
	aspect = [1, 0.21, 1];
      else
	aspect = [0.21, 1, 1];
      endif
    endif
  else
    aspect = NaN;
  endif

endfunction

%!demo
%! hold off;
%! close all;
%! n = 64; x = kron (1:n,ones(n,1)); x = abs(x - x.'); 
%! imagesc(x)
%! colorbar();

%!demo
%! hold off;
%! n = 64; x = kron (1:n,ones(n,1)); x = abs(x - x.'); 
%! imagesc(x)
%! colorbar("westoutside");

%!demo
%! hold off;
%! n = 64; x = kron (1:n,ones(n,1)); x = abs(x - x.'); 
%! imagesc(x)
%! colorbar("northoutside");

%!demo
%! hold off;
%! n = 64; x = kron (1:n,ones(n,1)); x = abs(x - x.'); 
%! imagesc(x)
%! colorbar("southoutside");

%!demo
%! hold off;
%! subplot(2,2,1)
%! contour(peaks())
%! colorbar("east");
%! subplot(2,2,2)
%! contour(peaks())
%! colorbar("west");
%! subplot(2,2,3)
%! contour(peaks())
%! colorbar("north");
%! subplot(2,2,4)
%! contour(peaks())
%! colorbar("south");

%!demo
%! hold off;
%! n = 64; x = kron (1:n,ones(n,1)); x = abs(x - x.'); 
%! subplot(2,2,1)
%! imagesc(x)
%! colorbar();
%! subplot(2,2,2)
%! imagesc(x)
%! colorbar("westoutside");
%! subplot(2,2,3)
%! imagesc(x)
%! colorbar("northoutside");
%! subplot(2,2,4)
%! imagesc(x)
%! colorbar("southoutside");

%!demo
%! hold off;
%! n = 64; x = kron (1:n,ones(n,1)); x = abs(x - x.'); 
%! subplot(1,2,1)
%! imagesc(x)
%! axis square;
%! colorbar();
%! subplot(1,2,2)
%! imagesc(x)
%! axis square;
%! colorbar("westoutside");

%!demo
%! hold off;
%! n = 64; x = kron (1:n,ones(n,1)); x = abs(x - x.'); 
%! subplot(1,2,1)
%! imagesc(x)
%! axis square;
%! colorbar("northoutside");
%! subplot(1,2,2)
%! imagesc(x)
%! axis square;
%! colorbar("southoutside");

%!demo
%! hold off;
%! n = 64; x = kron (1:n,ones(n,1)); x = abs(x - x.'); 
%! subplot(2,1,1)
%! imagesc(x)
%! axis square;
%! colorbar();
%! subplot(2,1,2)
%! imagesc(x)
%! axis square;
%! colorbar("westoutside");

%!demo
%! hold off;
%! n = 64; x = kron (1:n,ones(n,1)); x = abs(x - x.'); 
%! subplot(2,1,1)
%! imagesc(x)
%! axis square;
%! colorbar("northoutside");
%! subplot(2,1,2)
%! imagesc(x)
%! axis square;
%! colorbar("southoutside");

%!demo
%! hold off;
%! n = 64; x = kron (1:n,ones(n,1)); x = abs(x - x.'); 
%! subplot(1,2,1)
%! imagesc(x)
%! colorbar();
%! subplot(1,2,2)
%! imagesc(x)
%! colorbar("westoutside");

%!demo
%! hold off;
%! n = 64; x = kron (1:n,ones(n,1)); x = abs(x - x.'); 
%! subplot(1,2,1)
%! imagesc(x)
%! colorbar("northoutside");
%! subplot(1,2,2)
%! imagesc(x)
%! colorbar("southoutside");

%!demo
%! hold off;
%! n = 64; x = kron (1:n,ones(n,1)); x = abs(x - x.'); 
%! subplot(2,1,1)
%! imagesc(x)
%! colorbar();
%! subplot(2,1,2)
%! imagesc(x)
%! colorbar("westoutside");

%!demo
%! hold off;
%! n = 64; x = kron (1:n,ones(n,1)); x = abs(x - x.'); 
%! subplot(2,1,1)
%! imagesc(x)
%! colorbar("northoutside");
%! subplot(2,1,2)
%! imagesc(x)
%! colorbar("southoutside");


