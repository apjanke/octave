## Copyright (C) 1995, 1996, 1997, 2005, 2006, 2007 Kurt Hornik
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
## @deftypefn {Function File} {} t_rnd (@var{n}, @var{r}, @var{c})
## @deftypefnx {Function File} {} t_rnd (@var{n}, @var{sz})
## Return an @var{r} by @var{c} matrix of random samples from the t
## (Student) distribution with @var{n} degrees of freedom.  @var{n} must
## be a scalar or of size @var{r} by @var{c}. Or if @var{sz} is a
## vector create a matrix of size @var{sz}.
##
## If @var{r} and @var{c} are omitted, the size of the result matrix is
## the size of @var{n}.
## @end deftypefn

## Author: KH <Kurt.Hornik@wu-wien.ac.at>
## Description: Random deviates from the t distribution

## Deprecated in version 3.0

function rnd = t_rnd (varargin)

  persistent warned = false;
  if (! warned)
    warned = true;
    warning ("Octave:deprecated-function",
        ["t_rnd is obsolete and will be removed from a future\n",
	       "version of Octave, please use trnd instead"]);
  endif

 rnd =  trnd (varargin{:});

endfunction
