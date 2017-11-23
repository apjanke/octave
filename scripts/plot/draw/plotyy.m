## Copyright (C) 2007-2017 David Bateman
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
## @deftypefn  {} {} plotyy (@var{x1}, @var{y1}, @var{x2}, @var{y2})
## @deftypefnx {} {} plotyy (@dots{}, @var{fun})
## @deftypefnx {} {} plotyy (@dots{}, @var{fun1}, @var{fun2})
## @deftypefnx {} {} plotyy (@var{hax}, @dots{})
## @deftypefnx {} {[@var{ax}, @var{h1}, @var{h2}] =} plotyy (@dots{})
## Plot two sets of data with independent y-axes and a common x-axis.
##
## The arguments @var{x1} and @var{y1} define the arguments for the first plot
## and @var{x1} and @var{y2} for the second.
##
## By default the arguments are evaluated with
## @code{feval (@@plot, @var{x}, @var{y})}.  However the type of plot can be
## modified with the @var{fun} argument, in which case the plots are
## generated by @code{feval (@var{fun}, @var{x}, @var{y})}.  @var{fun} can be
## a function handle, an inline function, or a string of a function name.
##
## The function to use for each of the plots can be independently defined
## with @var{fun1} and @var{fun2}.
##
## If the first argument @var{hax} is an axes handle, then it defines
## the principal axis in which to plot the @var{x1} and @var{y1} data.
##
## The return value @var{ax} is a vector with the axis handles of the two
## y-axes.  @var{h1} and @var{h2} are handles to the objects generated by the
## plot commands.
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
## @seealso{plot}
## @end deftypefn

function [ax, h1, h2] = plotyy (varargin)

  [hax, varargin] = __plt_get_axis_arg__ ("plotyy", varargin{:});

  nargin = numel (varargin);
  if (nargin < 4 || nargin > 6)
    print_usage ();
  endif

  oldfig = [];
  if (! isempty (hax))
    oldfig = get (0, "currentfigure");
  endif
  unwind_protect
    hax = newplot (hax);

    ## FIXME: Second conditional test shouldn't be required.
    ##        'cla reset' needs to delete user properties like __plotyy_axes__.
    if (isprop (hax, "__plotyy_axes__")
        && isaxes (get (hax, "__plotyy_axes__")) == [true true])
      hax = get (hax, "__plotyy_axes__");
    else
      hax(2) = axes ("nextplot", get (hax(1), "nextplot"));
    endif

    [axtmp, h1tmp, h2tmp] = __plotyy__ (hax, varargin{:});

    set (gcf, "currentaxes", hax(1));

  unwind_protect_cleanup
    if (! isempty (oldfig))
      set (0, "currentfigure", oldfig);
    endif
  end_unwind_protect

  if (nargout > 0)
    ax = axtmp;
    h1 = h1tmp;
    h2 = h2tmp;
  endif

endfunction

function [ax, h1, h2] = __plotyy__ (ax, x1, y1, x2, y2, fun1 = @plot, fun2)

  if (nargin < 7)
    fun2 = fun1;
  endif

  xlim = [min([x1(:); x2(:)]), max([x1(:); x2(:)])];

  axes (ax(1));

  h1 = feval (fun1, x1, y1);

  set (ax(1), "ycolor", getcolor (h1(1)), "xlim", xlim);

  set (gcf (), "nextplot", "add");

  axes (ax(2));

  colors = get (ax(1), "colororder");
  set (ax(2), "colororder", [colors(2:end,:); colors(1,:)]);

  if (strcmp (get (ax(1), "autopos_tag"), "subplot"))
    set (ax(2), "autopos_tag", "subplot");
  else
    set (ax, "activepositionproperty", "position");
  endif

  ## Don't replace axis which has colororder property already modified
  if (strcmp (get (ax(1), "nextplot"), "replace"))
    set (ax(2), "nextplot", "replacechildren");
  endif
  h2 = feval (fun2, x2, y2);

  set (ax(2), "yaxislocation", "right", "color", "none",
              "ycolor", getcolor (h2(1)), "box", "off", "xlim", xlim);

  if (strcmp (get(ax(1), "activepositionproperty"), "position"))
    set (ax(2), "position", get (ax(1), "position"));
  else
    set (ax(2), {"outerposition", "looseinset"},
                get (ax(1), {"outerposition", "looseinset"}));
  endif

  ## Restore nextplot value by copying value from axis #1
  set (ax(2), "nextplot", get (ax(1), "nextplot"));

  ## Add invisible text objects that when destroyed,
  ## also remove the other axis
  t1 = text (0, 0, "", "parent", ax(1), "tag", "plotyy",
             "visible", "off", "handlevisibility", "off",
             "xliminclude", "off", "yliminclude", "off");
  t2 = text (0, 0, "", "parent", ax(2), "tag", "plotyy",
             "visible", "off", "handlevisibility", "off",
             "xliminclude", "off", "yliminclude", "off");

  set (t1, "deletefcn", {@deleteplotyy, ax(2), t2});
  set (t2, "deletefcn", {@deleteplotyy, ax(1), t1});

  ## Add cross-listeners so a change in one axes' attributes updates the other.
  addlistener (ax(1), "position", {@update_position, ax(2)});
  addlistener (ax(2), "position", {@update_position, ax(1)});
  addlistener (ax(1), "outerposition", {@update_position, ax(2)});
  addlistener (ax(2), "outerposition", {@update_position, ax(1)});
  addlistener (ax(1), "looseinset", {@update_position, ax(2)});
  addlistener (ax(2), "looseinset", {@update_position, ax(1)});
  addlistener (ax(1), "view", {@update_position, ax(2)});
  addlistener (ax(2), "view", {@update_position, ax(1)});
  addlistener (ax(1), "plotboxaspectratio", {@update_position, ax(2)});
  addlistener (ax(2), "plotboxaspectratio", {@update_position, ax(1)});
  addlistener (ax(1), "plotboxaspectratiomode", {@update_position, ax(2)});
  addlistener (ax(2), "plotboxaspectratiomode", {@update_position, ax(1)});
  addlistener (ax(1), "nextplot", {@update_nextplot, ax(2)});
  addlistener (ax(2), "nextplot", {@update_nextplot, ax(1)});

  ## Store the axes handles for the sister axes.
  if (ishandle (ax(1)) && ! isprop (ax(1), "__plotyy_axes__"))
    addproperty ("__plotyy_axes__", ax(1), "data");
    set (ax(1), "__plotyy_axes__", ax);
  elseif (ishandle (ax(1)))
    set (ax(1), "__plotyy_axes__", ax);
  else
    error ("plotyy.m: This shouldn't happen.  File a bug report.");
  endif
  if (ishandle (ax(2)) && ! isprop (ax(2), "__plotyy_axes__"))
    addproperty ("__plotyy_axes__", ax(2), "data");
    set (ax(2), "__plotyy_axes__", ax);
  elseif (ishandle (ax(2)))
    set (ax(2), "__plotyy_axes__", ax);
  else
    error ("plotyy.m: This shouldn't happen.  File a bug report.");
  endif

endfunction

function deleteplotyy (h, ~, ax2, t2)
  if (isaxes (ax2)
      && (isempty (gcbf ()) || strcmp (get (gcbf (), "beingdeleted"), "off"))
      && strcmp (get (ax2, "beingdeleted"), "off"))
    set (t2, "deletefcn", []);
    delete (ax2);
  endif
endfunction

function update_nextplot (h, ~, ax2)
  persistent recursion = false;

  if (! recursion)
    unwind_protect
      recursion = true;
      set (ax2, "nextplot", get (h, "nextplot"));
    unwind_protect_cleanup
      recursion = false;
    end_unwind_protect
  endif

endfunction

function update_position (h, ~, ax2)
  persistent recursion = false;

  ## Don't allow recursion
  if (! recursion)
    unwind_protect
      recursion = true;
      view = get (h, "view");
      oldview = get (ax2, "view");
      plotboxaspectratio = get (h, "plotboxaspectratio");
      oldplotboxaspectratio = get (ax2, "plotboxaspectratio");
      plotboxaspectratiomode = get (h, "plotboxaspectratiomode");
      oldplotboxaspectratiomode = get (ax2, "plotboxaspectratiomode");

      if (strcmp (get (h, "activepositionproperty"), "position"))
        position = get (h, "position");
        oldposition = get (ax2, "position");
        if (! (isequal (position, oldposition) && isequal (view, oldview)))
          set (ax2, "position", position, "view", view);
        endif
      else
        outerposition = get (h, "outerposition");
        oldouterposition = get (ax2, "outerposition");
        if (! (isequal (outerposition, oldouterposition)
               && isequal (view, oldview)))
          set (ax2, "outerposition", outerposition, "view", view);
        endif
      endif

      if (! (isequal (plotboxaspectratio, oldplotboxaspectratio)
             && isequal (plotboxaspectratiomode, oldplotboxaspectratiomode)))
        set (ax2, "plotboxaspectratio", plotboxaspectratio,
                  "plotboxaspectratiomode", plotboxaspectratiomode);
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
  elseif (isfield (obj, "edgecolor") && ! ischar (obj.edgecolor))
    color = obj.edgecolor;
  else
    color = [0, 0, 0];
  endif

endfunction


%!demo
%! clf;
%! x = 0:0.1:2*pi;
%! y1 = sin (x);
%! y2 = exp (x - 1);
%! ax = plotyy (x,y1, x-1,y2, @plot, @semilogy);
%! xlabel ("X");
%! ylabel (ax(1), "Axis 1");
%! ylabel (ax(2), "Axis 2");
%! lcolor = get (gca, "ColorOrder")(1,:);
%! rcolor = get (gca, "ColorOrder")(2,:);
%! text (0.5, 0.5, "Left Axis", ...
%!       "color", lcolor, "horizontalalignment", "center", "parent", ax(1));
%! text (4.5, 80, "Right Axis", ...
%!       "color", rcolor, "horizontalalignment", "center", "parent", ax(2));
%! title ({"plotyy() example"; "left axis uses @plot, right axis uses @semilogy"});

%!demo
%! clf;
%! colormap ("default");
%! x = linspace (-1, 1, 201);
%! subplot (2,2,1);
%!  plotyy (x,sin(pi*x), x,10*cos(pi*x));
%! subplot (2,2,2);
%!  surf (peaks (25));
%! subplot (2,2,3);
%!  contour (peaks (25));
%! subplot (2,2,4);
%!  plotyy (x,10*sin(2*pi*x), x,cos(2*pi*x));
%!  axis square;

%!demo
%! clf;
%! hold on
%! t = (0:0.1:9);
%! x = sin (t);
%! y = 5 * cos (t);
%! [hax, h1, h2] = plotyy (t, x, t, y);
%! [~, h3, h4] = plotyy (t+1, x, t+1, y);
%! set ([h3, h4], "linestyle", "--");
%! xlabel (hax(1), "xlabel");
%! title (hax(2), 'Two plotyy graphs on same figure using "hold on"');
%! ylabel (hax(1), "Left axis is Blue");
%! ylabel (hax(2), "Right axis is Orange");
