## Copyright (C) 2000, 2001, 2006, 2007, 2008, 2009 Kai Habel
## Copyright (C) 2006 David Bateman
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
## @deftypefn  {Function File} {@var{pp} =} spline (@var{x}, @var{y})
## @deftypefnx {Function File} {@var{yi} =} spline (@var{x}, @var{y}, @var{xi})
##
## Return the cubic spline interpolant of @var{y} at points @var{x}. 
## If called with two arguments, @code{spline} returns the piece-wise
## polynomial @var{pp} that may later be used with @code{ppval} to
## evaluate the polynomial at specific points.
## If called with a third input argument, @code{spline} evaluates the 
## spline at the points @var{xi}.  There is an equivalence
## between @code{ppval (spline (@var{x}, @var{y}), @var{xi})} and
## @code{spline (@var{x}, @var{y}, @var{xi})}.
##
## The variable @var{x} must be a vector of length @var{n}, and @var{y}
## can be either a vector or array.  In the case where @var{y} is a
## vector, it can have a length of either @var{n} or @code{@var{n} + 2}.
## If the length of @var{y} is @var{n}, then the 'not-a-knot' end
## condition is used.  If the length of @var{y} is @code{@var{n} + 2},
## then the first and last values of the vector @var{y} are the values
## of the first derivative of the cubic spline at the end-points.
##
## If @var{y} is an array, then the size of @var{y} must have the form
## @tex
## $$[s_1, s_2, \cdots, s_k, n]$$
## @end tex
## @ifnottex
## @code{[@var{s1}, @var{s2}, @dots{}, @var{sk}, @var{n}]}
## @end ifnottex
## or
## @tex
## $$[s_1, s_2, \cdots, s_k, n + 2].$$
## @end tex
## @ifnottex
## @code{[@var{s1}, @var{s2}, @dots{}, @var{sk}, @var{n} + 2]}.
## @end ifnottex
## The array is then reshaped internally to a matrix where the leading
## dimension is given by 
## @tex
## $$s_1 s_2 \cdots s_k$$
## @end tex
## @ifnottex
## @code{@var{s1} * @var{s2} * @dots{} * @var{sk}}
## @end ifnottex
## and each row of this matrix is then treated separately.  Note that this
## is exactly the opposite treatment than @code{interp1} and is done
## for compatibility.
## @seealso{ppval, mkpp, unmkpp}
## @end deftypefn

## This code is based on csape.m from octave-forge, but has been
## modified to use the sparse solver code in octave that itself allows
## special casing of tri-diagonal matrices, modified for NDArrays and
## for the treatment of vectors y 2 elements longer than x as complete
## splines.

function ret = spline (x, y, xi)

  x = x(:);
  n = length (x);
  if (n < 3)
    error ("spline: requires at least 3 points"); 
  endif

  ## Check the size and shape of y
  ndy = ndims (y);
  szy = size (y);
  if (ndy == 2 && (szy(1) == 1 || szy(2) == 1))
    if (szy(1) == 1)
      a = y.';
    else
      a = y;
      szy = fliplr (szy);
    endif
  else
    a = reshape (y, [prod(szy(1:end-1)), szy(end)]).';
  endif
  
  for k = (1:columns (a))(any (isnan (a))) 
    ok = ! isnan (a(:,k)); 
    a(!ok,k) = spline (x(ok), a(ok,k), x(!ok)); 
  endfor 
  
  complete = false;
  if (size (a, 1) == n + 2)
    complete = true;
    dfs = a(1,:);
    dfe = a(end,:);
    a = a(2:end-1,:);
  endif

  b = c = zeros (size (a));
  h = diff (x);
  idx = ones (columns (a), 1);

  if (complete)

    if (n == 3)
      dg = 1.5 * h(1) - 0.5 * h(2);
      c(2:n-1,:) = 1/dg(1);
    else
      dg = 2 * (h(1:n-2) .+ h(2:n-1));
      dg(1) = dg(1) - 0.5 * h(1);
      dg(n-2) = dg(n-2) - 0.5 * h(n-1);

      e = h(2:n-2);

      g = 3 * diff (a(2:n,:)) ./ h(2:n-1,idx) ...
          - 3 * diff (a(1:n-1,:)) ./ h(1:n-2,idx);
      g(1,:) = 3 * (a(3,:) - a(2,:)) / h(2) ...
          - 3 / 2 * (3 * (a(2,:) - a(1,:)) / h(1) - dfs);
      g(n-2,:) = 3 / 2 * (3 * (a(n,:) - a(n-1,:)) / h(n-1) - dfe) ...
          - 3 * (a(n-1,:) - a(n-2,:)) / h(n-2);

      c(2:n-1,:) = spdiags ([[e(:); 0], dg, [0; e(:)]],
                              [-1, 0, 1], n-2, n-2) \ g;
    endif

    c(1,:) = (3 / h(1) * (a(2,:) - a(1,:)) - 3 * dfs
              - c(2,:) * h(1)) / (2 * h(1));
    c(n,:) = - (3 / h(n-1) * (a(n,:) - a(n-1,:)) - 3 * dfe
                + c(n-1,:) * h(n-1)) / (2 * h(n-1));
    b(1:n-1,:) = diff (a) ./ h(1:n-1, idx) ...
        - h(1:n-1,idx) / 3 .* (c(2:n,:) + 2 * c(1:n-1,:));
    d = diff (c) ./ (3 * h(1:n-1, idx));

  else

    g = zeros (n-2, columns (a));
    g(1,:) = 3 / (h(1) + h(2)) ...
        * (a(3,:) - a(2,:) - h(2) / h(1) * (a(2,:) - a(1,:)));
    g(n-2,:) = 3 / (h(n-1) + h(n-2)) ...
        * (h(n-2) / h(n-1) * (a(n,:) - a(n-1,:)) - (a(n-1,:) - a(n-2,:)));

    if (n > 4)

      g(2:n - 3,:) = 3 * diff (a(3:n-1,:)) ./ h(3:n-2,idx) ...
          - 3 * diff (a(2:n-2,:)) ./ h(2:n - 3,idx);

      dg = 2 * (h(1:n-2) .+ h(2:n-1));
      dg(1) = dg(1) - h(1);
      dg(n-2) = dg(n-2) - h(n-1);

      ldg = udg = h(2:n-2);
      udg(1) = udg(1) - h(1);
      ldg(n - 3) = ldg(n-3) - h(n-1);
      c(2:n-1,:) = spdiags ([[ldg(:); 0], dg, [0; udg(:)]],
                              [-1, 0, 1], n-2, n-2) \ g;

    elseif (n == 4)

      dg = [h(1) + 2 * h(2); 2 * h(2) + h(3)];
      ldg = h(2) - h(3);
      udg = h(2) - h(1);
      c(2:n-1,:) = spdiags ([[ldg(:);0], dg, [0; udg(:)]],
                              [-1, 0, 1], n-2, n-2) \ g;
      
    else # n == 3
            
      dg = h(1) + 2 * h(2);
      c(2:n-1,:) = g/dg(1);

    endif

    c(1,:) = c(2,:) + h(1) / h(2) * (c(2,:) - c(3,:));
    c(n,:) = c(n-1,:) + h(n-1) / h(n-2) * (c(n-1,:) - c(n-2,:));
    b = diff (a) ./ h(1:n-1, idx) ...
        - h(1:n-1, idx) / 3 .* (c(2:n,:) + 2 * c(1:n-1,:));
    d = diff (c) ./ (3 * h(1:n-1, idx));

  endif

  d = d(1:n-1,:);
  c = c(1:n-1,:);
  b = b(1:n-1,:);
  a = a(1:n-1,:);
  coeffs = cat (3, d.', c.', b.', a.');
  ret = mkpp (x, coeffs, szy(1:end-1));

  if (nargin == 3)
    ret = ppval (ret, xi);
  endif

endfunction

%!demo
%! x = 0:10; y = sin(x);
%! xspline = 0:0.1:10; yspline = spline(x,y,xspline);
%! title("spline fit to points from sin(x)");
%! plot(xspline,sin(xspline),"r",xspline,yspline,"g-",x,y,"b+");
%! legend("original","interpolation","interpolation points");
%! %--------------------------------------------------------
%! % confirm that interpolated function matches the original

%!shared x,y,abserr
%! x = [0:10]; y = sin(x); abserr = 1e-14;
%!assert (spline(x,y,x), y, abserr);
%!assert (spline(x,y,x'), y', abserr);
%!assert (spline(x',y',x'), y', abserr);
%!assert (spline(x',y',x), y, abserr);
%!assert (isempty(spline(x',y',[])));
%!assert (isempty(spline(x,y,[])));
%!assert (spline(x,[y;y],x), [spline(x,y,x);spline(x,y,x)],abserr)
%! y = cos(x) + i*sin(x);
%!assert (spline(x,y,x), y, abserr)
%!assert (real(spline(x,y,x)), real(y), abserr);
%!assert (real(spline(x,y,x.')), real(y).', abserr);
%!assert (real(spline(x.',y.',x.')), real(y).', abserr);
%!assert (real(spline(x.',y,x)), real(y), abserr);
%!assert (imag(spline(x,y,x)), imag(y), abserr);
%!assert (imag(spline(x,y,x.')), imag(y).', abserr);
%!assert (imag(spline(x.',y.',x.')), imag(y).', abserr);
%!assert (imag(spline(x.',y,x)), imag(y), abserr);
%!test
%! xnan = 5;
%! y(x==xnan) = NaN;
%! ok = ! isnan (y);
%! assert (spline (x, y, x(ok)), y(ok), abserr);
%!test
%! ok = ! isnan (y);
%! assert (! isnan (spline (x, y, x(!ok))));
