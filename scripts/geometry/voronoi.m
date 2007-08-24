## Copyright (C) 2000  Kai Habel
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, write to the Free
## Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
## 02110-1301, USA.

## -*- texinfo -*-
## @deftypefn {Function File} {} voronoi (@var{x}, @var{y})
## @deftypefnx {Function File} {} voronoi (@var{x}, @var{y}, "plotstyle")
## @deftypefnx {Function File} {} voronoi (@var{x}, @var{y}, "plotstyle", @var{options})
## @deftypefnx {Function File} {[@var{vx}, @var{vy}] =} voronoi (@dots{})
## plots voronoi diagram of points @code{(@var{x}, @var{y})}.
## The voronoi facets with points at infinity are not drawn.
## [@var{vx}, @var{vy}] = voronoi(...) returns the vertices instead plotting the
## diagram. plot (@var{vx}, @var{vy}) shows the voronoi diagram.
##
## A fourth optional argument, which must be a string, contains extra options
## passed to the underlying qhull command.  See the documentation for the
## Qhull library for details.
##
## @example
## @group
##   x = rand (10, 1);
##   y = rand (size (x));
##   h = convhull (x, y);
##   [vx, vy] = voronoi (x, y);
##   plot (vx, vy, "-b", x, y, "o", x(h), y(h), "-g")
##   legend ("", "points", "hull");
## @end group
## @end example
##
## @seealso{voronoin, delaunay, convhull}
## @end deftypefn

## Author: Kai Habel <kai.habel@gmx.de>
## First Release: 20/08/2000

## 2002-01-04 Paul Kienzle <pkienzle@users.sf.net>
## * limit the default graph to the input points rather than the whole diagram
## * provide example
## * use unique(x,"rows") rather than __unique_rows__

## 2003-12-14 Rafael Laboissiere <rafael@laboissiere.net>
## Added optional fourth argument to pass options to the underlying
## qhull command

function [vvx, vvy] = voronoi (varargin)

  if (nargin < 1)
    print_usage ();
  endif

  narg = 1;
  if (isscalar (varargin{1}) && ishandle (varargin{1}))
    handl = varargin{1};
    narg++;
    if (! strcmp (get (handl, "type"), "axes"))
      error ("voronoi: expecting first argument to be an axes object");
    endif
  else
    if (nargout < 2)    
      handl = gca ();
    endif
  endif

  if (nargin < 1 + narg || nargin > 3 + narg)
    print_usage ();
  endif

  x = varargin{narg++};
  y = varargin{narg++};
  
  opts = {};
  if (narg <= nargin) 
    if (iscell (varargin{narg}))
      opts = varargin(narg++);
    elseif (ismatrix (varargin{narg}))
      ## Accept but ignore the triangulation
      narg++;
    endif
  endif

  linespec = {"b"};
  if (narg <= nargin) 
    if (ischar (varargin{narg}))
      linespec = varargin(narg);
    endif
  endif

  lx = length (x);
  ly = length (y);

  if (lx != ly)
    error ("voronoi: arguments must be vectors of same length");
  endif

  [p, c, infi] = __voronoi__ ([x(:),y(:)], opts{:});

  idx = find (!infi);

  ll = length (idx);
  k = 0;r = 1;

  for i = 1:ll
    k += length (c{idx(i)});
  endfor

  edges = zeros (2, k);

  for i=1:ll
    fac = c {idx(i)};
    lf = length(fac);
    fac = [fac, fac(1)];
    fst = fac(1:length(fac) - 1);
    sec = fac(2:length(fac));
    edges(:,r:r+lf-1) = [fst; sec];
    r += lf;
  endfor

  ## Identify the unique edges of the Voronoi diagram
  edges = sortrows (sort (edges).').';
  edges = edges (:,[(edges(1,1:end-1) != edges(1,2:end) | ...
		     edges(2,1:end-1) != edges(2,2:end)), true]);

  ## Get points of the diagram
  vx = reshape(p(edges, 1), size(edges));
  vy = reshape(p(edges, 2), size(edges));

  if (nargout < 2)    
    lim = [min(x(:)), max(x(:)), min(y(:)), max(y(:))];
    h = plot (handl, vx, vy, linespec{:}, x, y, '+');
    axis(lim+0.1*[[-1,1]*(lim(2)-lim(1)),[-1,1]*(lim(4)-lim(3))]);
    if (nargout == 1)
      vxx = h;
    endif
  elseif (nargout == 2)
    vvx = vx;
    vvy = vy;
  else
    error ("voronoi: only two or zero output arguments supported")
  endif

endfunction
