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
## @deftypefn {Function File} {} quiver (@var{u}, @var{v})
## @deftypefnx {Function File} {} quiver (@var{x}, @var{y}, @var{u}, @var{v})
## @deftypefnx {Function File} {} quiver (@dots{}, @var{s})
## @deftypefnx {Function File} {} quiver (@dots{}, @var{style})
## @deftypefnx {Function File} {} quiver (@dots{}, 'filled')
## @deftypefnx {Function File} {} quiver (@var{h}, @dots{})
## @deftypefnx {Function File} {@var{h} =} quiver (@dots{})
##
## Plot the @code{(@var{u}, @var{v})} components of a vector field in 
## an @code{(@var{x}, @var{y})} meshgrid. If the grid is uniform, you can 
## specify @var{x} and @var{y} as vectors.
##
## If @var{x} and @var{y} are undefined they are assumed to be
## @code{(1:@var{m}, 1:@var{n})} where @code{[@var{m}, @var{n}] = 
## size(@var{u})}.
##
## The variable @var{s} is a scalar defining a scaling factor to use for
##  the arrows of the field relative to the mesh spacing. A value of 0 
## disables all scaling. The default value is 1.
##
## The style to use for the plot can be defined with a line style @var{style}
## in a similar manner to the line styles used with the @code{plot} command.
## If a marker is specified then markers at the grid points of the vectors are
## printed rather than arrows. If the argument 'filled' is given then the
## markers as filled.
##
## The optional return value @var{h} provides a list of handles to the 
## the parts of the vector field (body, arrow and marker).
##
## @example
## @group
## [x, y] = meshgrid (1:2:20);
## quiver (x, y, sin (2*pi*x/10), sin (2*pi*y/10));
## @end group
## @end example
##
## @seealso{plot}
## @end deftypefn

function retval = quiver (varargin)

  if (nargin < 2)
    print_usage ();
  elseif (isscalar (varargin{1}) && ishandle (varargin{1}))
    h = varargin{1};
    if (! strcmp (get (h, "type"), "axes"))
      error ("quiver: expecting first argument to be an axes object");
    endif
    oldh = gca ();
    unwind_protect
      axes (h);
      newplot ();
      tmp = __quiver__ (h, 0, varargin{2:end});
    unwind_protect_cleanup
      axes (oldh);
    end_unwind_protect
  else
    newplot ();
    tmp = __quiver__ (gca (), 0, varargin{:});
  endif

  if (nargout > 0)
    retval = tmp;
  endif

endfunction

%!demo
%! [x,y] = meshgrid(1:2:20);
%! quiver(x,y,sin(2*pi*x/10),sin(2*pi*y/10))

%!demo
%! axis("equal");
%! x=linspace(0,3,80); y=sin(2*pi*x); theta=2*pi*x+pi/2;
%! quiver(x,y,sin(theta)/10,cos(theta)/10);
%! hold on; plot(x,y,"r"); hold off;
