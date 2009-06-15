## Copyright (C) 2007, 2008, 2009 David Bateman
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
## @deftypefn {Function File} {} plotyy (@var{x1}, @var{y1}, @var{x2}, @var{y2})
## @deftypefnx {Function File} {} plotyy (@dots{}, @var{fun})
## @deftypefnx {Function File} {} plotyy (@dots{}, @var{fun1}, @var{fun2})
## @deftypefnx {Function File} {} plotyy (@var{h}, @dots{})
## @deftypefnx {Function File} {[@var{ax}, @var{h1}, @var{h2}] =} plotyy (@dots{})
## Plots two sets of data with independent y-axes.  The arguments @var{x1} and
## @var{y1} define the arguments for the first plot and @var{x1} and @var{y2}
## for the second. 
##
## By default the arguments are evaluated with 
## @code{feval (@@plot, @var{x}, @var{y})}.  However the type of plot can be
## modified with the @var{fun} argument, in which case the plots are
## generated by @code{feval (@var{fun}, @var{x}, @var{y})}.  @var{fun} can be 
## a function handle, an inline function or a string of a function name.
##
## The function to use for each of the plots can be independently defined 
## with @var{fun1} and @var{fun2}.
##
## If given, @var{h} defines the principal axis in which to plot the @var{x1}
## and @var{y1} data.  The return value @var{ax} is a two element vector with
## the axis handles of the two plots.  @var{h1} and @var{h2} are handles to
## the objects generated by the plot commands.
##
## @example
## @group
## x = 0:0.1:2*pi; 
## y1 = sin (x);
## y2 = exp (x - 1);
## ax = plotyy (x, y1, x - 1, y2, @@plot, @@semilogy);
## xlabel ("X");
## ylabel (ax(1), "Axis 1");
## ylabel (ax(2), "Axis 2");
## @end group
## @end example
## @end deftypefn

function [Ax, H1, H2] = plotyy (varargin)

  ## Don't use __plt_get_axis_arg__ here as ax is a two vector for plotyy
  if (nargin > 1 && length (varargin{1}) == 2 && ishandle(varargin{1}(1)) 
      &&  ishandle(varargin{1}(2)) && 
      all (floor (varargin{1}) != varargin{1}))
    obj1 = get (varargin{1}(1));
    obj2 = get (varargin{1}(2));
    if (strcmp (obj1.type, "axes") || strcmp (obj2.type, "axes"))
      ax = [obj1, obj2];
      varargin(1) = [];
      if (isempty (varargin))
	varargin = {};
      endif
    else
      error ("plotyy: expecting first argument to be axes handle");
    endif
  else
    f = get (0, "currentfigure");
    if (isempty (f))
      f = figure ();
    endif
    ca = get (f, "currentaxes");
    if (isempty (ca))
      ax = [];
    elseif (strcmp (get (ca, "tag"), "plotyy"));
      ax = get (ca, "__plotyy_axes__");
    else
      ax = ca;
    endif
    if (length (ax) > 2)
      for i = 3 : length (ax)
        delete (ax (i));
      endfor
      ax = ax(1:2);
    elseif (length (ax) == 1)
      ax(2) = axes ();
    elseif (isempty (ax))
      ax(1) = axes ();
      ax(2) = axes ();
    endif
    if (nargin < 2)
      varargin = {};
    endif
  endif 

  if (nargin < 4)
    print_usage ();
  endif

  oldh = gca ();
  unwind_protect
    [ax, h1, h2] = __plotyy__ (ax, varargin{:});
  unwind_protect_cleanup
    ## Only change back to the old axis if we didn't delete it
    if (ishandle(oldh) && strcmp (get (oldh, "type"), "axes"))
      axes (oldh);
    endif
  end_unwind_protect

  if (nargout > 0)
    Ax = ax;
    H1 = h1;
    H2 = h2;
  endif

endfunction

function [ax, h1, h2] = __plotyy__ (ax, x1, y1, x2, y2, varargin)
  if (nargin > 5)
    fun1 = varargin{1};
  else
    fun1 = @plot;
  endif
  if (nargin > 6)
    fun2 = varargin{2};
  else
    fun2 = fun1;
  endif

  xlim = [min([x1(:); x2(:)]), max([x1(:); x2(:)])];

  if (ishandle(ax(1)) && strcmp (get (ax(1), "type"), "axes"))
    axes (ax(1));
  else
    ax(1) = axes ();
  endif
  newplot ();
  h1 = feval (fun1, x1, y1);

  set (ax(1), "ycolor", getcolor (h1(1)));
  set (ax(1), "xlim", xlim);

  cf = gcf ();
  set (cf, "nextplot", "add");

  if (ishandle(ax(2)) && strcmp (get (ax(2), "type"), "axes"))
    axes (ax(2));
  else
    ax(2) = axes ();
  endif
  newplot ();

  colors = get (ax(1), "colororder");
  set (ax(2), "colororder", [colors(2:end,:); colors(1,:)]);

  h2 = feval (fun2, x2, y2);
  set (ax(2), "yaxislocation", "right");
  set (ax(2), "ycolor", getcolor (h2(1)));
  set (ax(2), "position", get (ax(1), "position"));
  set (ax(2), "xlim", xlim);
  set (ax(2), "color", "none");

  ## Add invisible text objects that when destroyed, 
  ## also remove the other axis
  t1 = text (0, 0, "", "parent", ax(1), "tag", "plotyy", 
	     "handlevisibility", "off", "visible", "off",
	     "xliminclude", "off", "yliminclude", "off");
  t2 = text (0, 0, "", "parent", ax(2), "tag", "plotyy", 
	     "handlevisibility", "off", "visible", "off",
	     "xliminclude", "off", "yliminclude", "off");

  set (t1, "deletefcn", {@deleteplotyy, ax(2), t2});
  set (t2, "deletefcn", {@deleteplotyy, ax(1), t1});

  addlistener (ax(1), "position", {@update_position, ax(2)});
  addlistener (ax(2), "position", {@update_position, ax(1)});
  addlistener (ax(1), "view", {@update_position, ax(2)});
  addlistener (ax(2), "view", {@update_position, ax(1)});
  addlistener (ax(1), "dataaspectratio", {@update_position, ax(2)});
  addlistener (ax(2), "dataaspectratio", {@update_position, ax(1)});

  ## Tag the plotyy axes, so we can use that information
  ## not to mirror the y axis tick marks
  set (ax, "tag", "plotyy")

  ## Store the axes handles for the sister axes.
  addproperty ("__plotyy_axes__", ax(1), "data", ax);
  addproperty ("__plotyy_axes__", ax(2), "data", ax);

endfunction

%!demo
%! clf
%! x = 0:0.1:2*pi; 
%! y1 = sin (x);
%! y2 = exp (x - 1);
%! ax = plotyy (x, y1, x - 1, y2, @plot, @semilogy);
%! xlabel ("X");
%! ylabel (ax(1), "Axis 1");
%! ylabel (ax(2), "Axis 2");

%!demo
%! clf
%! x = linspace (-1, 1, 201);
%! subplot (2, 2, 1)
%! plotyy (x, sin(pi*x), x, 10*cos(pi*x))
%! subplot (2, 2, 2)
%! surf (peaks (25))
%! subplot (2, 2, 3)
%! contour (peaks (25))
%! subplot (2, 2, 4)
%! plotyy (x, 10*sin(2*pi*x), x, cos(2*pi*x))
%! axis square

function deleteplotyy (h, d, ax2, t2)
  if (ishandle (ax2) && strcmp (get (ax2, "type"), "axes") && 
      (isempty (gcbf()) || strcmp (get (gcbf(), "beingdeleted"),"off")) &&
      strcmp (get (ax2, "beingdeleted"), "off"))
    set (t2, "deletefcn", []);
    delete (ax2);
  endif
endfunction

function update_position (h, d, ax2)
  persistent recursion = false;

  ## Don't allow recursion
  if (! recursion)
    unwind_protect
      recursion = true;
      position = get (h, "position");
      view = get (h, "view");
      dataaspectratio = get (h, "dataaspectratio");
      oldposition = get (ax2, "position");
      oldview = get (ax2, "view");
      olddataaspectratio = get (ax2, "dataaspectratio");
      if (! (isequal (position, oldposition)
             && isequal (view, oldview)
             && isequal (dataaspectratio, olddataaspectratio)))
	set (ax2, "position", position,
                  "view", view,
		  "dataaspectratio", dataaspectratio);
      endif
    unwind_protect_cleanup
      recursion = false;
    end_unwind_protect
  endif  
endfunction

function color = getcolor (ax)
  obj = get (ax);
  if (isfield (obj, "color"))
    color = obj.color;
  elseif (isfield (obj, "facecolor") && ! ischar (obj.facecolor))
    color = obj.facecolor;
  elseif (isfield (obj, "edgecolor") && !  ischar (obj.edgecolor))
    color = obj.edgecolor;
  else
    color = [0, 0, 0];
  endif
endfunction

