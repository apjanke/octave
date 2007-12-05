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

function geometryimages (nm, typ)
  bury_output ();
  if (strcmp (typ, "png"))
    set (0, "defaulttextfontname", "*");
  endif
  if (isempty (findstr (octave_config_info ("DEFS"), "HAVE_QHULL"))
      && (strcmp (nm, "voronoi") || strcmp (nm, "griddata")
	  || strcmp (nm, "convhull") || strcmp (nm, "delaunay")
	  || strcmp (nm, "triplot")))
    sombreroimage (nm, typ);
  elseif (strcmp (nm, "voronoi"))
    rand("state",9);
    x = rand(10,1);
    y = rand(10,1);
    tri = delaunay (x, y);
    [vx, vy] = voronoi (x, y, tri);
    triplot (tri, x, y, "b");
    hold on;
    plot (vx, vy, "r");
    [r, c] = tri2circ (tri(end,:), x, y);
    pc = [-1:0.01:1];
    xc = r * sin(pi*pc) + c(1);
    yc = r * cos(pi*pc) + c(2);
    plot (xc, yc, "g-", "LineWidth", 3);
    axis([0, 1, 0, 1]);
    legend ("Delaunay Triangulation", "Voronoi Diagram");
    print (strcat (nm, ".", typ), strcat ("-d", typ))    
  elseif (strcmp (nm, "triplot"))
    rand ("state", 2)
    x = rand (20, 1);
    y = rand (20, 1);
    tri = delaunay (x, y);
    triplot (tri, x, y);
    print (strcat (nm, ".", typ), strcat ("-d", typ))    
  elseif (strcmp (nm, "griddata"))
    rand("state",1);
    x=2*rand(1000,1)-1;
    y=2*rand(size(x))-1;
    z=sin(2*(x.^2+y.^2));
    [xx,yy]=meshgrid(linspace(-1,1,32));
    griddata(x,y,z,xx,yy);
    print (strcat (nm, ".", typ), strcat ("-d", typ))    
  elseif (strcmp (nm, "convhull"))
    x = -3:0.05:3;
    y = abs (sin (x));
    k = convhull (x, y);
    plot (x(k),y(k),'r-',x,y,'b+');
    axis ([-3.05, 3.05, -0.05, 1.05]);
    print (strcat (nm, ".", typ), strcat ("-d", typ)) 
  elseif (strcmp (nm, "delaunay"))
    rand ("state", 1);
    x = rand (1, 10);
    y = rand (1, 10);
    T = delaunay (x, y);
    X = [ x(T(:,1)); x(T(:,2)); x(T(:,3)); x(T(:,1)) ];
    Y = [ y(T(:,1)); y(T(:,2)); y(T(:,3)); y(T(:,1)) ];
    axis ([0, 1, 0, 1]);
    plot(X, Y, "b", x, y, "r*");
    print (strcat (nm, ".", typ), strcat ("-d", typ)) 
  elseif (strcmp (nm, "inpolygon"))
    randn ("state", 2);
    x = randn (100, 1);
    y = randn (100, 1);
    vx = cos (pi * [-1 : 0.1: 1]);
    vy = sin (pi * [-1 : 0.1 : 1]);
    in = inpolygon (x, y, vx, vy);
    plot(vx, vy, x(in), y(in), "r+", x(!in), y(!in), "bo");
    axis ([-2, 2, -2, 2]);
    print (strcat (nm, ".", typ), strcat ("-d", typ)) 
  else
    error ("unrecognized plot requested");
  endif
  bury_output ();
endfunction

function [r, c] = tri2circ (tri, xx, yy)
  x = xx(tri);
  y = yy(tri);
  m = (y(1:end-1) - y(2:end)) ./ (x(1:end-1) - x(2:end));
  xc = (prod(m) .* (y(1) - y(end)) + m(end)*(x(1)+x(2)) - m(1)*(x(2)+x(3))) ...
        ./ (2 * (m(end) - m(1))); 
  yc = - (xc - (x(2) + x(3))./2) ./ m(end) + (y(2) + y(3)) / 2;
  c = [xc, yc];
  r = sqrt ((xc - x(1)).^2 + (yc - y(1)).^2);
endfunction

## Use this function before plotting commands and after every call to
## print since print() resets output to stdout (unfortunately, gnpulot
## can't pop output as it can the terminal type).
function bury_output ()
  f = figure (1);
  set (f, "visible", "off");
endfunction
function geometryimages (nm, typ)
  bury_output ();
  if (strcmp (nm, "voronoi"))
    rand("state",9);
    x = rand(10,1);
    y = rand(10,1);
    tri = delaunay (x, y);
    [vx, vy] = voronoi (x, y, tri);
    triplot (tri, x, y, "b");
    hold on;
    plot (vx, vy, "r");
    [r, c] = tri2circ (tri(end,:), x, y);
    pc = [-1:0.01:1];
    xc = r * sin(pi*pc) + c(1);
    yc = r * cos(pi*pc) + c(2);
    plot (xc, yc, "g-", "LineWidth", 3);
    axis([0, 1, 0, 1]);
    legend ("Delaunay Triangulation", "Voronoi Diagram");
    print (strcat (nm, ".", typ), strcat ("-d", typ))    
  elseif (strcmp (nm, "triplot"))
    rand ("state", 2)
    x = rand (20, 1);
    y = rand (20, 1);
    tri = delaunay (x, y);
    triplot (tri, x, y);
    print (strcat (nm, ".", typ), strcat ("-d", typ))    
  elseif (strcmp (nm, "griddata"))
    rand("state",1);
    x=2*rand(1000,1)-1;
    y=2*rand(size(x))-1;
    z=sin(2*(x.^2+y.^2));
    [xx,yy]=meshgrid(linspace(-1,1,32));
    griddata(x,y,z,xx,yy);
    print (strcat (nm, ".", typ), strcat ("-d", typ))    
  else
    error ("unrecognized plot requested");
  endif
  bury_output ();
endfunction

function [r, c] = tri2circ (tri, xx, yy)
  x = xx(tri);
  y = yy(tri);
  m = (y(1:end-1) - y(2:end)) ./ (x(1:end-1) - x(2:end));
  xc = (prod(m) .* (y(1) - y(end)) + m(end)*(x(1)+x(2)) - m(1)*(x(2)+x(3))) ...
        ./ (2 * (m(end) - m(1))); 
  yc = - (xc - (x(2) + x(3))./2) ./ m(end) + (y(2) + y(3)) / 2;
  c = [xc, yc];
  r = sqrt ((xc - x(1)).^2 + (yc - y(1)).^2);
endfunction

## Use this function before plotting commands and after every call to
## print since print() resets output to stdout (unfortunately, gnpulot
## can't pop output as it can the terminal type).
function bury_output ()
  f = figure (1);
  set (f, "visible", "off");
endfunction

function sombreroimage (nm, typ)
  if (strcmp (typ, "txt"))
    fid = fopen (sprintf ("%s.txt", nm), "wt");
    fputs (fid, "+-----------------------------+\n");
    fputs (fid, "| Image unavailable because   |\n");
    fputs (fid, "| of a missing QHULL library. |\n");
    fputs (fid, "+-----------------------------+\n");
    fclose (fid);
    return;
  else ## if (!strcmp (typ, "txt"))

    bury_output ();

    x = y = linspace (-8, 8, 41)';
    [xx, yy] = meshgrid (x, y);
    r = sqrt (xx .^ 2 + yy .^ 2) + eps;
    z = sin (r) ./ r;
    unwind_protect
      mesh (x, y, z);
      title ("Sorry, graphics not available because octave was\\ncompiled without the QHULL library.");
    unwind_protect_cleanup
      print (strcat (nm, ".", typ), strcat ("-d", typ));
      bury_output ();
    end_unwind_protect
  endif
endfunction
