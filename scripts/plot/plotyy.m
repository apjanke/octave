## Copyright (C) 2007 David Bateman
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
## Plots two sets of data with independent y-axes. The arguments @var{x1} and
## @var{y1} define the arguments for the first plot and @var{x1} and @var{y2}
## for the second. 
##
## By default the arguments are evaluated with 
## @code{feval (@@plot, @var{x}, @var{y})}. However the type of plot can be
## modified with the @var{fun} argument, in which case the plots are
## generated by @code{feval (@var{fun}, @var{x}, @var{y})}. @var{fun} can be 
## a function handle, an inline function or a string of a function name.
##
## The function to use for each of the plots can be independently defined 
## with @var{fun1} and @var{fun2}.
##
## If given, @var{h} defines the principal axis in which to plot the @var{x1}
## and @var{y1} data. The return value @var{ax} is a two element vector with
## the axis handles of the two plots. @var{h1} and @var{h2} are handles to
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

  [ax, varargin] = __plt_get_axis_arg__ ("plotyy", varargin{:});

  if (nargin < 4)
    print_usage ();
  endif

  oldh = gca ();
  unwind_protect
    axes (ax);
    newplot ();
    [ax, h1, h2] = __plotyy__ (ax, varargin{:});
  unwind_protect_cleanup
    axes (oldh);
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

  h1 = feval (fun1, x1, y1);

  set (ax(1), "ycolor", getcolor (h1(1)));
  set (ax(1), "position", get (ax(1), "outerposition"));
  set (ax(1), "xlim", xlim);

  cf = gcf ();
  set (cf, "nextplot", "add");
  ax(2) = axes ();
  colors = get (ax(1), "colororder");
  set (ax(2), "colororder", [colors(2:end,:); colors(1,:)]);

  h2 = feval (fun2, x2, y2);
  set (ax(2), "yaxislocation", "right");
  set (ax(2), "ycolor", getcolor (h2(1)));
  set (ax(2), "position", get (ax(1), "outerposition"));
  set (ax(2), "xlim", xlim);
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
