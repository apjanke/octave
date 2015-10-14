## Copyright (C) 2013 Roberto Porcu' <roberto.porcu@polimi.it>
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
## @deftypefn  {Function File} {@var{sum} =} kahan (@var{sum}, @var{comp}, @var{term})
## @deftypefnx {Function File} {[@var{sum}, @var{comp}] =} kahan (@var{sum}, @var{comp}, @var{term})
##
## This function implements the Kahan summation algorithm, also known as
## compensated summation.
##
## The algorithm significantly reduces the numerical error in the total
## obtained by adding a sequence of finite precision floating point numbers,
## compared to the straightforward approach.  For more details
## see @url{http://en.wikipedia.org/wiki/Kahan_summation_algorithm}.
## This function is called by @command{integrate_adaptive} and
## @command{integrate_const} to better catch equality comparisons.
##
## The first input argument is the variable that will contain the summation,
## so that is also returned as first output argument in order to reuse it in
## next calls to @code{kahan} function.
##
## The second input argument contains the compensation term and it is returned
## as the second output argument so that it can be reused in future calls of
## the same summation.
##
## The third input argument @var{term} is the variable to be added to @var{sum}.
## @end deftypefn

function [sum, comp] = kahan (sum, comp, term)

  y = term - comp;
  t = sum + y;
  comp = (t - sum) - y;
  sum = t;

endfunction

