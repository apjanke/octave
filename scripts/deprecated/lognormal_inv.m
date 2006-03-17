## Copyright (C) 1995, 1996, 1997  Kurt Hornik
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
## @deftypefn {Function File} {} lognormal_inv (@var{x}, @var{a}, @var{v})
## For each element of @var{x}, compute the quantile (the inverse of the
## CDF) at @var{x} of the lognormal distribution with parameters @var{a}
## and @var{v}.  If a random variable follows this distribution, its
## logarithm is normally distributed with mean @code{log (@var{a})} and
## variance @var{v}.
##
## Default values are @var{a} = 1, @var{v} = 1.
## @end deftypefn

## Author: KH <Kurt.Hornik@wu-wien.ac.at>
## Description: Quantile function of the log normal distribution

function inv = lognormal_inv (varargin)

  if (nargin > 1)
    a = varargin{2};
    idx = a >= 0;
    a(idx) = log (a(idx));
    a(!idx) = NaN;
    varargin{2} = a;
  endif

  if (nargin > 2)
    v = varargin{3};
    idx = v >= 0;
    v(idx) = sqrt (v(idx));
    v(!idx) = NaN;
    varargin{3} = v;
  endif

  inv = logninv (varargin{:});

endfunction
