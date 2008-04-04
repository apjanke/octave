## Copyright (C) 1995, 1996, 1997, 2005, 2007 Kurt Hornik
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
## @deftypefn {Function File} {} beta_rnd (@var{a}, @var{b}, @var{r}, @var{c})
## @deftypefnx {Function File} {} beta_rnd (@var{a}, @var{b}, @var{sz})
## Return an @var{r} by @var{c} or @code{size (@var{sz})} matrix of 
## random samples from the Beta distribution with parameters @var{a} and
## @var{b}.  Both @var{a} and @var{b} must be scalar or of size @var{r}
##  by @var{c}.
##
## If @var{r} and @var{c} are omitted, the size of the result matrix is
## the common size of @var{a} and @var{b}.
## @end deftypefn

## Author: KH <Kurt.Hornik@wu-wien.ac.at>
## Description: Random deviates from the Beta distribution

## Deprecated in version 3.0

function rnd = beta_rnd (varargin)

 rnd =  betarnd (varargin{:});

endfunction
