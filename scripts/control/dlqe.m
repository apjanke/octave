# Copyright (C) 1993, 1994, 1995 Auburn University.  All Rights Reserved.
# 
# This file is part of Octave.
# 
# Octave is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any
# later version.
# 
# Octave is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Octave; see the file COPYING.  If not, write to the Free
# Software Foundation, 59 Temple Place, Suite 330, Boston, MA 02111 USA.

## -*- texinfo -*-
## @deftypefn {Function File} {[@var{l}, @var{m}, @var{p}, @var{e}] =} dlqe (@var{a}, @var{g}, @var{c}, @var{sigw}, @var{sigv}, @var{z})
## Construct the linear quadratic estimator (Kalman filter) for the
## discrete time system
## @iftex
## @tex
## $$
##  x_{k+1} = A x_k + B u_k + G w_k
## $$
## $$
##  y_k = C x_k + D u_k + w_k
## $$
## @end tex
## @end iftex
## @ifinfo
## 
## @example
## x[k+1] = A x[k] + B u[k] + G w[k]
##   y[k] = C x[k] + D u[k] + w[k]
## @end example
## 
## @end ifinfo
## where @var{w}, @var{v} are zero-mean gaussian noise processes with
## respective intensities @code{@var{sigw} = cov (@var{w}, @var{w})} and
## @code{@var{sigv} = cov (@var{v}, @var{v})}.
## 
## If specified, @var{z} is @code{cov (@var{w}, @var{v})}.  Otherwise
## @code{cov (@var{w}, @var{v}) = 0}.
## 
## The observer structure is
## @iftex
## @tex
## $$
##  z_{k+1} = A z_k + B u_k + k (y_k - C z_k - D u_k)
## $$
## @end tex
## @end iftex
## @ifinfo
## 
## @example
## z[k+1] = A z[k] + B u[k] + k (y[k] - C z[k] - D u[k])
## @end example
## @end ifinfo
## 
## @noindent
## The following values are returned:
## 
## @table @var
## @item l
## The observer gain, 
## @iftex
## @tex
## $(A - ALC)$.
## @end tex
## @end iftex
## @ifinfo
## (@var{a} - @var{a}@var{l}@var{c}).
## @end ifinfo
## is stable.
## 
## @item m
## The Riccati equation solution.
## 
## @item p
## The estimate error covariance after the measurement update.
## 
## @item e
## The closed loop poles of
## @iftex
## @tex
## $(A - ALC)$.
## @end tex
## @end iftex
## @ifinfo
## (@var{a} - @var{a}@var{l}@var{c}).
## @end ifinfo
## @end table
## @end deftypefn

function [l, m, p, e] = dlqe (a, g, c, sigw, sigv, s)
# Written by A. S. Hodel (scotte@eng.auburn.edu) August, 1993.
# Modified for discrete time by R. Bruce Tenison (btenison@eng.auburn.edu)
# October, 1993

  if (nargin != 5 && nargin != 6)
    error ("dlqe: invalid number of arguments");
  endif

# The problem is dual to the regulator design, so transform to dlqr call.

  if (nargin == 5)
    [k, p, e] = dlqr (a', c', g*sigw*g', sigv);
    m = p;
    l = k';
  else
    [k, p, e] = dlqr (a', c', g*sigw*g', sigv, g*s);
    m = p;
    l = k';
    a = a-g*t/sigv*c;
    sigw = sigw-t/sigv;
  endif

  p = a\(m-g*sigw*g')/a';

endfunction
